# ============================================================
# Sims-Style Cartoon Voice Generator — Gender/Type Naming
# (with parametrizable final volume for all voices)
# ============================================================
# Generates trimmed, punchy syllables with voices labeled as
# 'woman', 'man', and 'deep' (not persons). File names are:
#   woman-ba.wav, man-ba.wav, deep-ba.wav, etc.
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

# Folder locations
SYLLABLES_FOLDER = Path("syllables")
VOICES_FOLDER = Path("voices")
TARGET_FINAL_DURATION_MS = 200  # Each output clip: 200ms

# Simple English syllables — one per file
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

# Voice types instead of personas
VOICE_TYPES = {
    "woman": {
        "pitch_semitones": +4,
        "speed_factor": 1.15,
        "volume_change_db": 8,
        "reverb": False,
        "glitch": 0.04,
        "distortion": 0.02,
        "final_volume_db": -8
    },
    "man": {
        "pitch_semitones": -3,
        "speed_factor": 1.15,
        "volume_change_db": 8,
        "reverb": False,
        "glitch": 0.05,
        "distortion": 0.03,
        "final_volume_db": -8
    },
    "deep": {
        "pitch_semitones": -7,
        "speed_factor": 1.15,
        "volume_change_db": 9,
        "reverb": True,
        "glitch": 0.15,
        "distortion": 0.07,
        "final_volume_db": -8
    },
}

def decode_mp3_basic(mp3_bytes):
    """Decode MP3 file to AudioSegment."""
    try:
        return AudioSegment.from_mp3(io.BytesIO(mp3_bytes))
    except Exception as e:
        print(f"[ERROR] MP3 decoding: {e}")
        return None

def aggressive_trim_silence(audio, silence_thresh=-40, chunk_size=10):
    """
    Aggressive silence trimming; removes attack and release, keeps only core.
    """
    nonsilent_ranges = silence.detect_nonsilent(audio, min_silence_len=chunk_size, silence_thresh=silence_thresh)
    if not nonsilent_ranges:
        return audio
    start_trim, end_trim = nonsilent_ranges[0][0], nonsilent_ranges[-1][1]
    audio_core = audio[start_trim:end_trim].fade_in(5).fade_out(5)
    return audio_core

def generate_base_syllables():
    """
    Generate basic syllable WAVs, trimmed aggressively, one pronunciation per syllable.
    """
    print("=== Generating base syllables ===\n")
    SYLLABLES_FOLDER.mkdir(exist_ok=True)
    for name, text in BASE_SYLLABLES.items():
        output_path = SYLLABLES_FOLDER / f"{name}.wav"
        if output_path.exists():
            print(f"[SKIP] {name}.wav")
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
            audio = aggressive_trim_silence(audio, silence_thresh=-40)
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
    """Synthetic harmonic distortion effect."""
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
    """Glitch: random amplitude drops."""
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

def add_reverb(audio, amount=0.15):
    """Short digital echo, low amount."""
    delay = 30
    if len(audio) <= delay:
        return audio
    try:
        delayed = AudioSegment.silent(duration=delay) + audio[:-delay]
        return audio.overlay(delayed, position=0, gain_during_overlay=amount - 7)
    except:
        return audio

def process_syllable(syllable_name, voicetype, params):
    """
    Convert one syllable to one voice type with all effects,
    outputting as <type>-<syllable>.wav in 'voices' folder.
    """
    in_path = SYLLABLES_FOLDER / f"{syllable_name}.wav"
    if not in_path.exists():
        print(f"[WARNING] Missing: {in_path}")
        return
    try:
        audio = AudioSegment.from_wav(str(in_path))
        if len(audio) == 0:
            print(f"[ERROR] Empty audio: {in_path}")
            return
        # Speed up first to increase sense of urgency
        speed_factor = params.get("speed_factor", 1.0)
        if speed_factor != 1.0:
            audio = audio.speedup(playback_speed=speed_factor)
        # Professional pitch shift
        audio = pitch_shift_librosa(audio, params.get("pitch_semitones", 0))
        # Aggressively trim leftover silences after effects
        audio = aggressive_trim_silence(audio, silence_thresh=-40)
        # End trimming: hard cut to exact target duration
        if len(audio) > TARGET_FINAL_DURATION_MS:
            audio = audio[:TARGET_FINAL_DURATION_MS]
        elif len(audio) < TARGET_FINAL_DURATION_MS:
            audio += AudioSegment.silent(TARGET_FINAL_DURATION_MS - len(audio))
        # Fade out 5ms to prevent clicks
        audio = audio.fade_out(5)
        # Volume
        audio += params.get("volume_change_db", 0)
        # Effects
        audio = add_distortion(audio, params.get("distortion", 0))
        audio = add_glitch(audio, params.get("glitch", 0))
        if params.get("reverb", False):
            audio = add_reverb(audio, amount=0.15)
        audio = audio.normalize()
        # Final output volume adjustment (parametrizable)
        final_db = params.get("final_volume_db", 0)  # default = no change
        audio = audio + final_db if final_db != 0 else audio
        # Export with type name: woman-ba.wav etc
        out_path = VOICES_FOLDER / f"{voicetype}-{syllable_name}.wav"
        audio.export(str(out_path), format="wav")
        print(f"[OK] {out_path} ({len(audio)}ms)")
    except Exception as e:
        print(f"[ERROR] {syllable_name}, type={voicetype}: {e}")

if __name__ == "__main__":
    VOICES_FOLDER.mkdir(exist_ok=True)
    SYLLABLES_FOLDER.mkdir(exist_ok=True)
    generate_base_syllables()
    print("\n" + "=" * 60)
    print("Processing syllables into voice types")
    print("=" * 60 + "\n")
    for voicetype, params in VOICE_TYPES.items():
        print(f"\n--- {voicetype.upper()} ---\n")
        for syllable in BASE_SYLLABLES.keys():
            process_syllable(syllable, voicetype, params)
    print("\n" + "=" * 60)
    print("COMPLETE!")
    print("=" * 60)
