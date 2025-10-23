# ============================================================
# Sims-Style Cartoon Voice Generator - Tight & Fast Edition
# ============================================================
# Generates short, punchy syllables with aggressive trimming.
# No attack/decay, designed for seamless concatenation.
#
# Folders:
#   - "syllables" for base TTS WAVs
#   - "voices" for processed character voices
#
# Requirements:
#   pip install gtts pydub librosa soundfile numpy
#
# ============================================================

import numpy as np
from pydub import AudioSegment, silence
from pathlib import Path
from gtts import gTTS
import io
import librosa

# -------------------------
# CONFIGURATION
# -------------------------

SYLLABLES_FOLDER = Path("syllables")
VOICES_FOLDER = Path("voices")
TARGET_FINAL_DURATION_MS = 200  # Shorter for faster pace

# Single clear pronunciation
BASE_SYLLABLES = {
    "ba": "bah",
    "da": "dah",
    "ma": "mah",
    "na": "nah",
    "bi": "bee",
    "mi": "mee",
    "bo": "boh",
    "pa": "pah",
}

# Voice parameters with speed factor added
VOICES = {
    "linus": {
        "pitch_semitones": -3,
        "speed_factor": 1.15,    # Speed up 15%
        "volume_change_db": 8,
        "reverb": False,
        "glitch": 0.05,
        "distortion": 0.03,
    },
    "clio": {
        "pitch_semitones": +4,
        "speed_factor": 1.15,    # Speed up 15%
        "volume_change_db": 9,
        "reverb": False,
        "glitch": 0.04,
        "distortion": 0.02,
    },
    "xander": {
        "pitch_semitones": -7,
        "speed_factor": 1.15,    # Speed up 15%
        "volume_change_db": 9,
        "reverb": True,
        "glitch": 0.15,
        "distortion": 0.07,
    },
}

# -------------------------
# HELPER FUNCTIONS
# -------------------------

def decode_mp3_basic(mp3_bytes):
    """Decode MP3 to pydub AudioSegment."""
    try:
        return AudioSegment.from_mp3(io.BytesIO(mp3_bytes))
    except Exception as e:
        print(f"[ERROR] MP3 decoding: {e}")
        return None

def aggressive_trim_silence(audio, silence_thresh=-40, chunk_size=10):
    """
    Aggressively trim silence and fade edges.
    Removes attack and decay for tight syllables.
    """
    try:
        # Detect non-silent parts with aggressive threshold
        nonsilent_ranges = silence.detect_nonsilent(
            audio, 
            min_silence_len=chunk_size,
            silence_thresh=silence_thresh
        )
        
        if not nonsilent_ranges:
            return audio
        
        # Get core audio only
        start_trim = nonsilent_ranges[0][0]
        end_trim = nonsilent_ranges[-1][1]
        
        # Cut MORE aggressively - remove attack/decay completely
        audio_core = audio[start_trim:end_trim]
        
        # Apply very short fade in/out (5ms) to avoid clicks
        audio_core = audio_core.fade_in(5).fade_out(5)
        
        return audio_core
    except Exception as e:
        print(f"[WARNING] Trim failed: {e}")
        return audio

def generate_base_syllables():
    """Generate clean syllables with aggressive trimming."""
    print("=== Generating base syllables (tight & clean) ===\n")
    SYLLABLES_FOLDER.mkdir(exist_ok=True)
    
    for name, text in BASE_SYLLABLES.items():
        output_path = SYLLABLES_FOLDER / f"{name}.wav"
        if output_path.exists():
            print(f"[SKIP] {name}.wav exists")
            continue
        
        try:
            print(f"[GENERATING] {name} ('{text}')...", end=" ", flush=True)
            tts = gTTS(text=text, lang="en", slow=False)
            mp3_buffer = io.BytesIO()
            tts.write_to_fp(mp3_buffer)
            mp3_buffer.seek(0)
            
            audio = decode_mp3_basic(mp3_buffer.getvalue())
            if audio is None:
                print("[ERROR] Empty audio")
                continue
            
            # Aggressively trim all silence
            audio = aggressive_trim_silence(audio, silence_thresh=-40)
            
            # Normalize
            audio = audio.normalize()
            
            audio.export(str(output_path), format="wav")
            print(f"[OK] {len(audio)}ms")
        except Exception as e:
            print(f"[ERROR] {e}")

def pitch_shift_librosa(audio_segment, semitones):
    """High-quality pitch shifting."""
    if semitones == 0:
        return audio_segment
    
    try:
        samples = np.array(audio_segment.get_array_of_samples(), dtype=np.float32)
        if audio_segment.channels == 2:
            samples = samples.reshape((-1, 2)).mean(axis=1)
        samples = samples / 32768.0
        
        sr = audio_segment.frame_rate
        shifted = librosa.effects.pitch_shift(samples, sr=sr, n_steps=semitones)
        
        # Normalize
        peak = np.max(np.abs(shifted))
        if peak > 0:
            shifted = shifted / peak * 0.95
        
        samples_int16 = np.int16(shifted * 32767)
        
        return AudioSegment(
            samples_int16.tobytes(),
            frame_rate=sr,
            sample_width=2,
            channels=1
        )
    except Exception as e:
        print(f"[WARNING] Pitch shift failed: {e}")
        return audio_segment

def add_distortion(audio, amount):
    """Subtle harmonic distortion."""
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
        return AudioSegment(samples_int16.tobytes(), frame_rate=audio.frame_rate,
                            sample_width=2, channels=1)
    except:
        return audio

def add_glitch(audio, amount):
    """Random amplitude drops."""
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
        return AudioSegment(samples_int16.tobytes(), frame_rate=audio.frame_rate,
                            sample_width=2, channels=1)
    except:
        return audio

def add_reverb(audio, amount=0.15):
    """Very short echo for minimal reverb."""
    delay = 30  # Shorter delay for tighter sound
    if len(audio) <= delay:
        return audio
    try:
        delayed = AudioSegment.silent(duration=delay) + audio[:-delay]
        return audio.overlay(delayed, position=0, gain_during_overlay=amount - 7)
    except:
        return audio

# -------------------------
# MAIN PROCESSING
# -------------------------

def process_syllable(syllable_name, voice_name, params, index):
    """Process syllable into tight character voice."""
    in_path = SYLLABLES_FOLDER / f"{syllable_name}.wav"
    if not in_path.exists():
        print(f"[WARNING] Missing: {in_path}")
        return
    
    try:
        audio = AudioSegment.from_wav(str(in_path))
        if len(audio) == 0:
            print(f"[ERROR] Empty audio: {in_path}")
            return
        
        # Speed up audio first (makes it shorter and snappier)
        speed_factor = params.get("speed_factor", 1.0)
        if speed_factor != 1.0:
            audio = audio.speedup(playback_speed=speed_factor)
        
        # Apply pitch shift
        semitones = params.get("pitch_semitones", 0)
        audio = pitch_shift_librosa(audio, semitones)
        
        # Trim or pad to exact target (tight!)
        if len(audio) > TARGET_FINAL_DURATION_MS:
            # Trim from end, keep the core syllable
            audio = audio[:TARGET_FINAL_DURATION_MS]
        elif len(audio) < TARGET_FINAL_DURATION_MS:
            # Pad minimally at end
            audio += AudioSegment.silent(TARGET_FINAL_DURATION_MS - len(audio))
        
        # Add very short fade at end to avoid click when concatenating
        audio = audio.fade_out(5)
        
        # Volume boost
        audio += params.get("volume_change_db", 0)
        
        # Apply effects
        audio = add_distortion(audio, params.get("distortion", 0))
        audio = add_glitch(audio, params.get("glitch", 0))
        
        if params.get("reverb", False):
            audio = add_reverb(audio, amount=0.15)
        
        # Final normalization
        audio = audio.normalize()
        
        # Export
        out_path = VOICES_FOLDER / f"{voice_name}voice{index + 1}.wav"
        audio.export(str(out_path), format="wav")
        print(f"[OK] {out_path} ({len(audio)}ms)")
        
    except Exception as e:
        print(f"[ERROR] {syllable_name}: {e}")

# -------------------------
# MAIN EXECUTION
# -------------------------

if __name__ == "__main__":
    VOICES_FOLDER.mkdir(exist_ok=True)
    SYLLABLES_FOLDER.mkdir(exist_ok=True)
    
    generate_base_syllables()
    
    print("\n" + "=" * 60)
    print("Processing syllables into character voices")
    print("=" * 60 + "\n")
    
    for voice_name, params in VOICES.items():
        print(f"\n--- {voice_name.upper()} ---\n")
        for idx, syllable in enumerate(BASE_SYLLABLES.keys()):
            process_syllable(syllable, voice_name, params, idx)
    
    print("\n" + "=" * 60)
    print("COMPLETE!")
    print("=" * 60)
