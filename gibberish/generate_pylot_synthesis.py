#!/usr/bin/env python3
# ============================================================
# SYLLABLE DECOMPOSITION WITH FREE AI + PHONETIC MAPPING
# Maps AI-generated syllables to closest inventory match
# ============================================================

import requests
import json
import numpy as np
from pydub import AudioSegment, silence
from pathlib import Path
from gtts import gTTS
import io
import librosa
import unicodedata
import time
from difflib import SequenceMatcher

# ============================================================
# CONFIGURATION
# ============================================================

OPENROUTER_API_KEY = "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"
OPENROUTER_MODEL = "mistralai/mistral-7b-instruct"

GLOBAL_SPEED_MULTIPLIER = 1.4
TRIM_PERCENTAGE = 0.05
FADE_IN_MS = 15
FADE_OUT_MS = 80
SYLLABLE_MIN_VOLUME_FACTOR = 0.3
SILENCE_THRESHOLD_DB = -35
MIN_SILENCE_LEN_MS = 5
LOWPASS_CUTOFF_HZ = 3500
BASE_VOLUME_REDUCTION_DB = -6
NORMALIZATION_PEAK = 0.95
INTER_SYLLABLE_SILENCE_MS = 10

SYLLABLES_FOLDER = Path("syllables_pilot")
VOICES_FOLDER = Path("voices_pilot")
SYNTHESIS_FOLDER = Path("synthesis_test")

# ============================================================
# SYLLABLE INVENTORY (40 SYLLABLES)
# ============================================================

SYLLABLE_MAP = {
    "pa": ("pa", "pah"), "pe": ("pe", "peh"), "pi": ("pi", "pee"), "po": ("po", "poh"), "pu": ("pu", "poo"),
    "ta": ("ta", "tah"), "te": ("te", "teh"), "ti": ("ti", "tee"), "to": ("to", "toh"), "tu": ("tu", "too"),
    "ka": ("ka", "kah"), "ke": ("ke", "keh"), "ki": ("ki", "kee"), "ko": ("ko", "koh"), "ku": ("ku", "koo"),
    "ba": ("ba", "bah"), "be": ("be", "beh"), "bi": ("bi", "bee"), "bo": ("bo", "boh"), "bu": ("bu", "boo"),
    "da": ("da", "dah"), "de": ("de", "deh"), "di": ("di", "dee"), "do": ("do", "doh"), "du": ("du", "doo"),
    "ga": ("ga", "gah"), "ge": ("ge", "geh"), "gi": ("gi", "gee"), "go": ("go", "goh"), "gu": ("gu", "goo"),
    "sa": ("sa", "sah"), "se": ("se", "seh"), "si": ("si", "see"), "so": ("so", "soh"), "su": ("su", "soo"),
    "fa": ("fa", "fah"), "fe": ("fe", "feh"), "fi": ("fi", "fee"), "fo": ("fo", "foh"), "fu": ("fu", "foo"),
}

SYLLABLE_INDICES = {key: idx for idx, key in enumerate(sorted(SYLLABLE_MAP.keys()))}
SYLLABLE_LIST = sorted(SYLLABLE_MAP.keys())

# Consonant-to-consonant and vowel-to-vowel approximation tables
CONSONANT_MAP = {
    # Map any consonant to closest in inventory
    'p': 'p', 'b': 'b', 'm': 'p',      # Bilabials
    't': 't', 'd': 'd', 'n': 't',      # Alveolars
    'k': 'k', 'g': 'g',                # Velars
    'f': 'f', 'v': 'f', 's': 's',      # Fricatives
    'z': 's', 'x': 's', 'j': 's',      # More fricatives
    'l': 'p', 'r': 't', 'w': 'b',      # Approximants
    'y': 'p', 'h': 'f', 'c': 't',      # Special
}

VOWEL_MAP = {
    # Map any vowel to closest in inventory (a, e, i, o, u)
    'a': 'a', 'á': 'a', 'à': 'a',
    'e': 'e', 'é': 'e', 'è': 'e',
    'i': 'i', 'í': 'i', 'ì': 'i',
    'o': 'o', 'ó': 'o', 'ò': 'o',
    'u': 'u', 'ú': 'u', 'ù': 'u',
    'y': 'i',  # y → i
    'w': 'u',  # w → u
}

VOICE_TYPES = {
    "woman": {
        "pitch_semitones": +3,
        "speed_factor_multiplier": 1.0,
        "volume_change_db": 6,
        "glitch": 0.03,
        "distortion": 0.02,
        "final_volume_db": BASE_VOLUME_REDUCTION_DB,
        "lowpass_cutoff": 3200,
    },
    "man": {
        "pitch_semitones": -2,
        "speed_factor_multiplier": 1.0,
        "volume_change_db": 6,
        "glitch": 0.04,
        "distortion": 0.03,
        "final_volume_db": BASE_VOLUME_REDUCTION_DB,
        "lowpass_cutoff": 3500,
    },
    "deep": {
        "pitch_semitones": -6,
        "speed_factor_multiplier": 1.0,
        "volume_change_db": 7,
        "glitch": 0.12,
        "distortion": 0.06,
        "final_volume_db": BASE_VOLUME_REDUCTION_DB,
        "lowpass_cutoff": 3800,
    },
}

TEST_SENTENCES = [
    {
        "id": "test_1_overslept",
        "es": "Creo que he dormido demasiado",
        "en": "I think I've overslept",
        "voice": "man",
    },
    {
        "id": "test_2_mother",
        "es": "Perdóname maestro",
        "en": "Forgive me master",
        "voice": "man",
    },
    {
        "id": "test_3_island",
        "es": "La isla del Gremio de los Tejedores",
        "en": "The Weavers guild island",
        "voice": "deep",
    },
]

# ============================================================
# PHONETIC APPROXIMATION FUNCTIONS
# ============================================================

def approximate_syllable_phonetic(syllable):
    """
    Maps any syllable to closest CV pair in inventory using phonetic rules.
    
    Example:
        "cre" → "ka" (c→k, e→e, ignore r)
        "que" → "ke" (q→k, u→e, u→e)
        "don" → "do" (d→d, o→o, ignore n)
        "rmi" → "pi" (r→p, m→p, i→i)
    """
    
    if len(syllable) == 0:
        return "pa"  # Default fallback
    
    syllable = syllable.lower()
    
    # Extract first consonant and first vowel from the syllable
    consonant = None
    vowel = None
    
    for char in syllable:
        if consonant is None and char in CONSONANT_MAP:
            consonant = CONSONANT_MAP[char]
        elif vowel is None and char in VOWEL_MAP:
            vowel = VOWEL_MAP[char]
    
    # If we found both, combine them
    if consonant and vowel:
        result = consonant + vowel
        if result in SYLLABLE_MAP:
            return result
    
    # Fallback 1: consonant + default vowel 'a'
    if consonant:
        result = consonant + 'a'
        if result in SYLLABLE_MAP:
            return result
    
    # Fallback 2: default consonant 'p' + vowel
    if vowel:
        result = 'p' + vowel
        if result in SYLLABLE_MAP:
            return result
    
    # Fallback 3: just use closest match by string similarity
    closest = min(SYLLABLE_LIST, key=lambda x: SequenceMatcher(None, syllable, x).ratio())
    return closest

def approximate_syllables_smart(syllables):
    """
    Takes a list of syllables from AI and approximates each to inventory.
    """
    approximated = []
    for syl in syllables:
        if syl in SYLLABLE_MAP:
            # Already valid
            approximated.append(syl)
        else:
            # Approximate
            approx = approximate_syllable_phonetic(syl)
            approximated.append(approx)
            print(f"      '{syl}' → '{approx}'")
    
    return approximated

# ============================================================
# AI-BASED SYLLABLE MAPPING WITH APPROXIMATION
# ============================================================

def syllabify_with_ai(text, language='es'):
    """
    Uses Mistral 7B to decompose text into syllables,
    then approximates them to inventory.
    """
    
    available_syllables = ", ".join(sorted(SYLLABLE_MAP.keys()))
    
    prompt = f"""You are a phonetic expert. Decompose this {language} text into SYLLABLES (not phonemes, actual syllables as spoken):

"{text}"

Return ONLY a JSON array like: ["syl", "la", "ble"]

Example for "hello": ["hel", "lo"]
Example for "casa": ["ca", "sa"]

Return ONLY JSON, no explanation:"""

    try:
        headers = {
            "Authorization": f"Bearer {OPENROUTER_API_KEY}",
            "Content-Type": "application/json",
        }
        
        payload = {
            "model": OPENROUTER_MODEL,
            "messages": [
                {"role": "user", "content": prompt}
            ],
            "temperature": 0.3,
            "max_tokens": 256,
        }
        
        response = requests.post(
            "https://openrouter.ai/api/v1/chat/completions",
            headers=headers,
            json=payload,
            timeout=30
        )
        
        if response.status_code != 200:
            print(f"[ERROR] OpenRouter API error: {response.status_code}")
            return []
        
        result = response.json()
        response_text = result["choices"][0]["message"]["content"].strip()
        
        print(f"    AI response: {response_text}")
        
        # Extract JSON array
        try:
            syllables = json.loads(response_text)
            if not isinstance(syllables, list):
                print(f"[ERROR] Not a list: {response_text}")
                return []
            
            # CRITICAL: Approximate each syllable to inventory
            print(f"    Approximating:")
            approximated = approximate_syllables_smart(syllables)
            
            return [(syl, SYLLABLE_INDICES[syl]) for syl in approximated]
        
        except json.JSONDecodeError:
            print(f"[ERROR] Could not parse JSON: {response_text}")
            return []
    
    except requests.exceptions.RequestException as e:
        print(f"[ERROR] Request failed: {e}")
        return []

# ============================================================
# HELPER FUNCTIONS (same as before)
# ============================================================

def decode_mp3_basic(mp3_bytes):
    try:
        return AudioSegment.from_mp3(io.BytesIO(mp3_bytes))
    except Exception as e:
        print(f"[ERROR] MP3 decoding: {e}")
        return None

def gentle_trim_silence(audio, silence_thresh=SILENCE_THRESHOLD_DB, chunk_size=15):
    nonsilent_ranges = silence.detect_nonsilent(audio, min_silence_len=chunk_size, silence_thresh=silence_thresh)
    if not nonsilent_ranges:
        return audio
    start_trim, end_trim = nonsilent_ranges[0][0], nonsilent_ranges[-1][1]
    start_trim = max(0, start_trim - 30)
    end_trim = min(len(audio), end_trim + 15)
    audio_core = audio[start_trim:end_trim]
    return audio_core.fade_in(5).fade_out(5)

def pitch_shift_librosa(audio_segment, semitones):
    if semitones == 0:
        return audio_segment
    try:
        samples = np.array(audio_segment.get_array_of_samples(), dtype=np.float32)
        if audio_segment.channels == 2:
            samples = samples.reshape((-1, 2)).mean(axis=1)
        samples = samples / 32768.0
        sr = audio_segment.frame_rate
        shifted = librosa.effects.pitch_shift(samples, sr=sr, n_steps=semitones)
        peak = np.max(np.abs(shifted))
        if peak > 0:
            shifted = shifted / peak * NORMALIZATION_PEAK
        samples_int16 = np.int16(shifted * 32767)
        return AudioSegment(samples_int16.tobytes(), frame_rate=sr, sample_width=2, channels=1)
    except Exception as e:
        print(f"[WARNING] Pitch shift failed: {e}")
        return audio_segment

def gentle_trim_cv(audio, trim_pct=TRIM_PERCENTAGE, fade_in_ms=FADE_IN_MS):
    nonsilent_ranges = silence.detect_nonsilent(audio, min_silence_len=MIN_SILENCE_LEN_MS, silence_thresh=SILENCE_THRESHOLD_DB)
    if not nonsilent_ranges:
        return audio
    start_trim = nonsilent_ranges[0][0]
    end_trim = nonsilent_ranges[-1][1]
    duration = end_trim - start_trim
    trim_margin = int(duration * trim_pct)
    end_trim = max(start_trim + int(duration * 0.3), end_trim - trim_margin)
    audio_core = audio[start_trim:end_trim]
    return audio_core.fade_in(fade_in_ms)

def apply_smooth_release(audio, min_factor=SYLLABLE_MIN_VOLUME_FACTOR, fade_length_ms=FADE_OUT_MS):
    samples = np.array(audio.get_array_of_samples())
    n_channels = audio.channels
    sample_count = len(samples) // n_channels if n_channels > 1 else len(samples)
    sr = audio.frame_rate
    fade_len = int(sr * fade_length_ms / 1000)
    max_fade_len = int(sample_count * 0.4)
    fade_len = min(fade_len, max_fade_len)
    if fade_len <= 0:
        return audio
    for c in range(n_channels):
        chan_offset = c if n_channels > 1 else 0
        chan_samples = samples[chan_offset::n_channels].copy()
        fade_start = len(chan_samples) - fade_len
        if fade_start < 0:
            fade_start = 0
        ramp = np.linspace(1.0, min_factor, fade_len)
        chan_samples[fade_start:] = (chan_samples[fade_start:] * ramp).astype(chan_samples.dtype)
        if n_channels > 1:
            samples[chan_offset::n_channels] = chan_samples
        else:
            samples = chan_samples
    return audio._spawn(samples.tobytes())

def add_lowpass_filter(audio, cutoff_freq=LOWPASS_CUTOFF_HZ):
    try:
        return audio.low_pass_filter(cutoff_freq)
    except:
        return audio

def add_distortion(audio, amount):
    if amount <= 0:
        return audio
    try:
        samples = np.array(audio.get_array_of_samples(), dtype=np.float32)
        if audio.channels == 2:
            samples = samples.reshape((-1, 2)).mean(axis=1)
        samples /= 32768.0
        distorted = np.tanh(samples * (1 + amount * 3)) * 0.85
        blended = samples * (1 - amount * 0.5) + distorted * (amount * 0.5)
        samples_int16 = np.int16(blended * 32767)
        return AudioSegment(samples_int16.tobytes(), frame_rate=audio.frame_rate, sample_width=2, channels=1)
    except:
        return audio

def add_glitch(audio, amount):
    if amount <= 0:
        return audio
    try:
        samples = np.array(audio.get_array_of_samples(), dtype=np.float32)
        if audio.channels == 2:
            samples = samples.reshape((-1, 2)).mean(axis=1)
        np.random.seed(42)
        mask = np.random.random(len(samples)) < (amount * 0.02)
        samples[mask] *= 0.7
        samples_int16 = np.int16(samples)
        return AudioSegment(samples_int16.tobytes(), frame_rate=audio.frame_rate, sample_width=2, channels=1)
    except:
        return audio

def create_silence(duration_ms, frame_rate=44100):
    num_samples = int(frame_rate * duration_ms / 1000)
    samples = np.zeros(num_samples, dtype=np.int16)
    return AudioSegment(samples.tobytes(), frame_rate=frame_rate, sample_width=2, channels=1)

def concatenate_syllables_with_gaps(syllable_list, all_syllables, voice_type, inter_silence_ms=INTER_SYLLABLE_SILENCE_MS):
    result = AudioSegment.empty()
    silence_gap = create_silence(inter_silence_ms)
    for i, (syl_text, syl_idx) in enumerate(syllable_list):
        if syl_text in all_syllables and voice_type in all_syllables[syl_text]:
            syl_audio = all_syllables[syl_text][voice_type]
            result += syl_audio
            if i < len(syllable_list) - 1:
                result += silence_gap
    return result

# ============================================================
# MAIN EXECUTION
# ============================================================

def main():
    SYLLABLES_FOLDER.mkdir(exist_ok=True)
    VOICES_FOLDER.mkdir(exist_ok=True)
    SYNTHESIS_FOLDER.mkdir(exist_ok=True)
    
    print("\n" + "="*60)
    print("SYLLABLE SYNTHESIS - AI + PHONETIC APPROXIMATION")
    print("="*60 + "\n")
    
    # STEP 1: Generate base syllables
    print("[1/4] Generating base syllables (40 total)...\n")
    for syl_text in sorted(SYLLABLE_MAP.keys()):
        output_path = SYLLABLES_FOLDER / f"{syl_text}.wav"
        if output_path.exists():
            continue
        try:
            _, phonetic = SYLLABLE_MAP[syl_text]
            print(f"  [GEN] {syl_text} ({phonetic})...", end=" ", flush=True)
            tts = gTTS(text=phonetic, lang="en", slow=False)
            mp3_buffer = io.BytesIO()
            tts.write_to_fp(mp3_buffer)
            mp3_buffer.seek(0)
            audio = decode_mp3_basic(mp3_buffer.getvalue())
            if audio is None:
                print("[ERROR]")
                continue
            audio = gentle_trim_silence(audio)
            audio = audio.normalize()
            audio.export(str(output_path), format="wav")
            print(f"[OK]")
        except Exception as e:
            print(f"[ERROR] {e}")
    
    # STEP 2: Process syllables for all voices
    print("\n[2/4] Processing syllables for 3 voices...\n")
    all_syllables = {}
    
    for voice_type, params in VOICE_TYPES.items():
        print(f"  Processing {voice_type.upper()}...")
        for syl_text in sorted(SYLLABLE_MAP.keys()):
            in_path = SYLLABLES_FOLDER / f"{syl_text}.wav"
            if not in_path.exists():
                continue
            
            audio = AudioSegment.from_wav(str(in_path))
            speed_factor = GLOBAL_SPEED_MULTIPLIER * params.get("speed_factor_multiplier", 1.0)
            if speed_factor != 1.0:
                audio = audio.speedup(playback_speed=speed_factor)
            
            audio = pitch_shift_librosa(audio, params.get("pitch_semitones", 0))
            audio = gentle_trim_cv(audio)
            audio += params.get("volume_change_db", 0)
            audio = add_distortion(audio, params.get("distortion", 0))
            audio = add_glitch(audio, params.get("glitch", 0))
            audio = add_lowpass_filter(audio, params.get("lowpass_cutoff", LOWPASS_CUTOFF_HZ))
            audio = apply_smooth_release(audio)
            audio = audio.normalize()
            
            final_db = params.get("final_volume_db", BASE_VOLUME_REDUCTION_DB)
            if final_db != 0:
                audio = audio + final_db
            
            if syl_text not in all_syllables:
                all_syllables[syl_text] = {}
            all_syllables[syl_text][voice_type] = audio
            
            out_path = VOICES_FOLDER / f"{voice_type}_{syl_text}.wav"
            audio.export(str(out_path), format="wav")
    
    print(f"  ✓ Generated {len(all_syllables)} syllables × {len(VOICE_TYPES)} voices")
    
    # STEP 3: Syllabify with AI + approximation
    print("\n[3/4] Syllabifying with AI + Approximation...\n")
    
    for test in TEST_SENTENCES:
        print(f"  {test['id']}:")
        print(f"    ES: {test['es']}")
        syl_es = syllabify_with_ai(test['es'], 'Spanish')
        print(f"    → {' + '.join(s[0] for s in syl_es)}")
        
        print(f"    EN: {test['en']}")
        syl_en = syllabify_with_ai(test['en'], 'English')
        print(f"    → {' + '.join(s[0] for s in syl_en)}\n")
        
        time.sleep(1)
    
    # STEP 4: Synthesize
    print("[4/4] Synthesizing...\n")
    
    for test in TEST_SENTENCES:
        voice = test['voice']
        print(f"  Synthesizing {test['id']} ({voice})...")
        
        syl_es = syllabify_with_ai(test['es'], 'Spanish')
        audio_es = concatenate_syllables_with_gaps(syl_es, all_syllables, voice)
        out_path_es = SYNTHESIS_FOLDER / f"{test['id']}_es.wav"
        audio_es.export(str(out_path_es), format="wav")
        print(f"    ✓ {out_path_es} ({len(audio_es)}ms)")
        
        syl_en = syllabify_with_ai(test['en'], 'English')
        audio_en = concatenate_syllables_with_gaps(syl_en, all_syllables, voice)
        out_path_en = SYNTHESIS_FOLDER / f"{test['id']}_en.wav"
        audio_en.export(str(out_path_en), format="wav")
        print(f"    ✓ {out_path_en} ({len(audio_en)}ms)")
        
        time.sleep(1)
    
    print("\n" + "="*60)
    print("✓ SYNTHESIS COMPLETE")
    print("="*60 + "\n")

if __name__ == "__main__":
    main()
