# ============================================================
# Sims-Style Cartoon Voice Generator — Numeric Indexed Edition
# ============================================================
# Generates voice files with numeric indices instead of syllable names.
# Easy to swap syllable content without changing game code.
# Format: [voicetype]_[index].wav (e.g., woman_0.wav, man_3.wav)
#
# Requirements:
#   pip install gtts pydub librosa soundfile numpy
# ============================================================

import numpy as np
from pydub import AudioSegment, silence
from pathlib import Path
from gtts import gTTS
import io
import librosa

# ============================================================
# GLOBAL TUNING PARAMETERS
# ============================================================

#GLOBAL_SPEED_MULTIPLIER = 2.8
GLOBAL_SPEED_MULTIPLIER = 1.4
TRIM_PERCENTAGE = 0.15
FADE_IN_MS = 30
FADE_OUT_MS = 5
CROSSFADE_MS = 15
SILENCE_THRESHOLD_DB = -35
MIN_SILENCE_LEN_MS = 5
LOWPASS_CUTOFF_HZ = 3500
BASE_VOLUME_REDUCTION_DB = -6
NORMALIZATION_PEAK = 0.95

SYLLABLES_FOLDER = Path("syllables")
VOICES_FOLDER = Path("voices")

# Map phoneme index to TTS text
# Change these strings to modify what sounds are generated
# Example: "bla" for "bla bla bla" effect
SYLLABLE_MAP = {
    0: "blabla",      # Index 0
    1: "blah",      # Index 1
    2: "blahblah",      # Index 2
    3: "bleh",      # Index 3
    4: "baah",      # Index 4
    5: "bli",      # Index 5
    6: "bleh",      # Index 6
    7: "blu",      # Index 7
}

# Alternative syllable set for "bla bla" effect (uncomment to use)
# SYLLABLE_MAP = {
#     0: "bla",
#     1: "bla",
#     2: "bla",
#     3: "bla",
#     4: "bla",
#     5: "bla",
#     6: "bla",
#     7: "bla",
# }

VOICE_TYPES = {
    "woman": {
        "pitch_semitones": +3,
        "speed_factor_multiplier": 1.0,
        "volume_change_db": 6,
        "glitch": 0.03,
        "distortion": 0.02,
        "final_volume_db": BASE_VOLUME_REDUCTION_DB,
        "lowpass_cutoff": 3200,
        "crossfade_enable": True,
    },
    "man": {
        "pitch_semitones": -2,
        "speed_factor_multiplier": 1.0,
        "volume_change_db": 6,
        "glitch": 0.04,
        "distortion": 0.03,
        "final_volume_db": BASE_VOLUME_REDUCTION_DB,
        "lowpass_cutoff": 3500,
        "crossfade_enable": True,
    },
    "deep": {
        "pitch_semitones": -6,
        "speed_factor_multiplier": 1.0,
        "volume_change_db": 7,
        "glitch": 0.12,
        "distortion": 0.06,
        "final_volume_db": BASE_VOLUME_REDUCTION_DB,
        "lowpass_cutoff": 3800,
        "crossfade_enable": True,
    },
}

def decode_mp3_basic(mp3_bytes):
    """Decode MP3 file to AudioSegment."""
    try:
        return AudioSegment.from_mp3(io.BytesIO(mp3_bytes))
    except Exception as e:
        print(f"[ERROR] MP3 decoding: {e}")
        return None

def gentle_trim_silence(audio, silence_thresh=SILENCE_THRESHOLD_DB, chunk_size=15):
    """Gentler silence trimming that preserves envelope."""
    nonsilent_ranges = silence.detect_nonsilent(
        audio, 
        min_silence_len=chunk_size, 
        silence_thresh=silence_thresh
    )
    if not nonsilent_ranges:
        return audio
    start_trim, end_trim = nonsilent_ranges[0][0], nonsilent_ranges[-1][1]
    start_trim = max(0, start_trim - 15)
    end_trim = min(len(audio), end_trim + 15)
    audio_core = audio[start_trim:end_trim]
    audio_core = audio_core.fade_in(10).fade_out(10)
    return audio_core

def generate_base_syllables():
    """Generate basic syllable WAVs using numeric indices."""
    print("=== Generating optimized syllables ===\n")
    SYLLABLES_FOLDER.mkdir(exist_ok=True)
    for index, text in SYLLABLE_MAP.items():
        output_path = SYLLABLES_FOLDER / f"{index}.wav"
        if output_path.exists():
            print(f"[SKIP] {index}.wav")
            continue
        try:
            print(f"[GENERATING] index {index} ('{text}')...", end=" ", flush=True)
            tts = gTTS(text=text, lang="en", slow=False)
            mp3_buffer = io.BytesIO()
            tts.write_to_fp(mp3_buffer)
            mp3_buffer.seek(0)
            audio = decode_mp3_basic(mp3_buffer.getvalue())
            if audio is None:
                print("[ERROR] Empty audio")
                continue
            audio = gentle_trim_silence(audio, silence_thresh=SILENCE_THRESHOLD_DB)
            audio = audio.normalize()
            audio.export(str(output_path), format="wav")
            print(f"[OK] {len(audio)}ms")
        except Exception as e:
            print(f"[ERROR] {e}")

def pitch_shift_librosa(audio_segment, semitones):
    """High-quality pitch shifting using librosa."""
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
        return AudioSegment(
            samples_int16.tobytes(),
            frame_rate=sr,
            sample_width=2,
            channels=1
        )
    except Exception as e:
        print(f"[WARNING] Pitch shift failed: {e}")
        return audio_segment

def aggressive_trim_cv(audio, trim_pct=TRIM_PERCENTAGE, fade_in_ms=FADE_IN_MS, fade_out_ms=FADE_OUT_MS):
    """Ultra-aggressive trim with smooth fade-in and clipped fade-out."""
    nonsilent_ranges = silence.detect_nonsilent(
        audio, 
        min_silence_len=MIN_SILENCE_LEN_MS,
        silence_thresh=SILENCE_THRESHOLD_DB
    )
    if not nonsilent_ranges:
        return audio
    
    start_trim = nonsilent_ranges[0][0]
    end_trim = nonsilent_ranges[-1][1]
    
    duration = end_trim - start_trim
    trim_margin = int(duration * trim_pct)
    
    start_trim = max(0, start_trim + trim_margin)
    end_trim = min(len(audio), end_trim - trim_margin)
    
    audio_core = audio[start_trim:end_trim]
    audio_core = audio_core.fade_in(fade_in_ms).fade_out(fade_out_ms)
    
    return audio_core

def apply_crossfade_prep(audio, crossfade_ms=CROSSFADE_MS):
    """Prepare audio for crossfading with next syllable."""
    return audio

def add_lowpass_filter(audio, cutoff_freq=LOWPASS_CUTOFF_HZ):
    """Apply low-pass filter to reduce high-frequency harshness."""
    try:
        filtered = audio.low_pass_filter(cutoff_freq)
        return filtered
    except Exception as e:
        print(f"[WARNING] Low-pass filter failed: {e}")
        return audio

def add_distortion(audio, amount):
    """Minimal harmonic distortion for synthetic character."""
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
    except Exception as e:
        print(f"[WARNING] Distortion failed: {e}")
        return audio

def add_glitch(audio, amount):
    """Subtle glitch effect for digital character."""
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
    except Exception as e:
        print(f"[WARNING] Glitch failed: {e}")
        return audio

def process_syllable(index, voicetype, params):
    """Process syllable with advanced concatenation optimization."""
    in_path = SYLLABLES_FOLDER / f"{index}.wav"
    if not in_path.exists():
        print(f"[WARNING] Missing: {in_path}")
        return
    try:
        audio = AudioSegment.from_wav(str(in_path))
        if len(audio) == 0:
            print(f"[ERROR] Empty audio: {in_path}")
            return
        
        speed_factor = GLOBAL_SPEED_MULTIPLIER * params.get("speed_factor_multiplier", 1.0)
        if speed_factor != 1.0:
            audio = audio.speedup(playback_speed=speed_factor)
        
        audio = pitch_shift_librosa(audio, params.get("pitch_semitones", 0))
        
        audio = aggressive_trim_cv(
            audio, 
            trim_pct=TRIM_PERCENTAGE,
            fade_in_ms=FADE_IN_MS,
            fade_out_ms=FADE_OUT_MS
        )
        
        if params.get("crossfade_enable", True):
            audio = apply_crossfade_prep(audio, crossfade_ms=CROSSFADE_MS)
        
        audio += params.get("volume_change_db", 0)
        
        audio = add_distortion(audio, params.get("distortion", 0))
        audio = add_glitch(audio, params.get("glitch", 0))
        
        lowpass_freq = params.get("lowpass_cutoff", LOWPASS_CUTOFF_HZ)
        audio = add_lowpass_filter(audio, lowpass_freq)
        
        audio = audio.normalize()
        
        final_db = params.get("final_volume_db", BASE_VOLUME_REDUCTION_DB)
        if final_db != 0:
            audio = audio + final_db
        
        # Numeric filename: voicetype_index.wav
        out_path = VOICES_FOLDER / f"{voicetype}_{index}.wav"
        audio.export(str(out_path), format="wav")
        print(f"[OK] {out_path} ({len(audio)}ms)")
        
    except Exception as e:
        print(f"[ERROR] index {index}, type={voicetype}: {e}")

if __name__ == "__main__":
    VOICES_FOLDER.mkdir(exist_ok=True)
    SYLLABLES_FOLDER.mkdir(exist_ok=True)
    generate_base_syllables()
    print("\n" + "=" * 60)
    print(f"Processing syllables (Global Speed: {GLOBAL_SPEED_MULTIPLIER}×)")
    print(f"Fade-in: {FADE_IN_MS}ms | Fade-out: {FADE_OUT_MS}ms | Crossfade: {CROSSFADE_MS}ms")
    print("=" * 60 + "\n")
    for voicetype, params in VOICE_TYPES.items():
        print(f"\n--- {voicetype.upper()} ---\n")
        for index in SYLLABLE_MAP.keys():
            process_syllable(index, voicetype, params)
    print("\n" + "=" * 60)
    print(f"COMPLETE! Generated 24 voice files (speed={GLOBAL_SPEED_MULTIPLIER}×)")
    print("Numeric format: [voicetype]_[index].wav")
    print("=" * 60)
