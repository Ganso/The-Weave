# ============================================================
# Simple Cartoon Voice Generator - Sims Style
# ============================================================
# Load syllables, apply pitch/speed independently, reduce duration to 0.1s.
# Add subtle Sims-like effects (slight distortion, glitch, reverb).
#
# Requirements:
#   pip install gtts pydub soundfile numpy
#
# ============================================================

import numpy as np
from pydub import AudioSegment
from pathlib import Path
from gtts import gTTS
import io

# -------------------------
# CONFIGURATION
# -------------------------
PHONEME_FOLDER = Path("phonems")
OUTPUT_FOLDER = Path(".")
TARGET_DURATION_MS = 100  # 0.1 seconds (reduced from 0.2)

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

VOICES = {
    "linus": {
        "speed_factor": 0.97,
        "pitch_shift": 0.90,
        "volume_change_db": 7,
        "reverb": False,
        "glitch": 0.1,           # subtle glitch
        "distortion": 0.05,      # minimal distortion
    },
    "clio": {
        "speed_factor": 0.92,
        "pitch_shift": 1.20,
        "volume_change_db": 8,
        "reverb": False,
        "glitch": 0.08,
        "distortion": 0.04,
    },
    "xander": {
        "speed_factor": 0.98,
        "pitch_shift": 0.70,
        "volume_change_db": 5,
        "reverb": True,
        "glitch": 0.15,          # more glitch for character
        "distortion": 0.08,      # more distortion
    },
}


# -------------------------
# TTS GENERATION
# -------------------------
def decode_mp3_basic(mp3_bytes):
    """Decode MP3 using pydub."""
    try:
        audio = AudioSegment.from_mp3(io.BytesIO(mp3_bytes))
        return audio
    except Exception as e:
        print(f"[ERROR] MP3 decoding: {e}")
        return None


def generate_base_syllables():
    """Generate base syllable WAV files using gtts."""
    print("=== Generating base syllables with phonetic TTS ===\n")
    
    PHONEME_FOLDER.mkdir(exist_ok=True)
    
    for internal_name, phonetic_text in BASE_SYLLABLES.items():
        output_path = PHONEME_FOLDER / f"{internal_name}.wav"
        
        if output_path.exists():
            print(f"[SKIP] {internal_name}.wav already exists")
            continue
        
        try:
            print(f"[GENERATING] {internal_name} ('{phonetic_text}')...", end=" ", flush=True)
            
            tts = gTTS(text=phonetic_text, lang='en', slow=False)
            mp3_buffer = io.BytesIO()
            tts.write_to_fp(mp3_buffer)
            mp3_buffer.seek(0)
            
            audio = decode_mp3_basic(mp3_buffer.getvalue())
            
            if audio is None:
                print(f"[ERROR] Empty audio data")
                continue
            
            audio.export(str(output_path), format="wav")
            print(f"[OK]")
            
        except Exception as e:
            print(f"[ERROR] {e}")


# -------------------------
# DSP EFFECTS - SUBTLE SIMS STYLE
# -------------------------
def change_pitch(audio, pitch_shift):
    """Change pitch without changing speed/duration."""
    if pitch_shift == 1.0:
        return audio
    
    try:
        sample_rate = audio.frame_rate
        new_frame_rate = int(sample_rate * pitch_shift)
        
        if new_frame_rate < 8000:
            new_frame_rate = 8000
        if new_frame_rate > 48000:
            new_frame_rate = 48000
        
        shifted_audio = audio._spawn(
            audio.raw_data,
            overrides={"frame_rate": new_frame_rate}
        ).set_frame_rate(sample_rate)
        
        return shifted_audio
    except Exception as e:
        print(f"    [WARNING] Pitch shift failed: {e}")
        return audio


def add_subtle_distortion(audio, amount):
    """Add subtle harmonic distortion for synthetic character."""
    if amount <= 0:
        return audio
    
    try:
        # Convert to numpy
        samples = np.array(audio.get_array_of_samples(), dtype=np.float32)
        if audio.channels == 2:
            samples = samples.reshape((-1, 2)).mean(axis=1)
        samples = samples / 32768.0
        
        # Apply soft clipping (tanh distortion)
        distorted = np.tanh(samples * (1 + amount * 2)) * 0.9
        
        # Blend: keep mostly original, add small amount of distortion
        samples_out = samples * (1 - amount * 0.5) + distorted * (amount * 0.5)
        
        # Convert back
        samples_int16 = np.int16(samples_out * 32767)
        audio_out = AudioSegment(
            samples_int16.tobytes(),
            frame_rate=audio.frame_rate,
            sample_width=2,
            channels=1
        )
        return audio_out
    except Exception as e:
        print(f"    [WARNING] Distortion failed: {e}")
        return audio


def add_subtle_glitch(audio, amount):
    """Add subtle glitch artifacts for Sims-like effect."""
    if amount <= 0:
        return audio
    
    try:
        samples = np.array(audio.get_array_of_samples(), dtype=np.float32)
        if audio.channels == 2:
            samples = samples.reshape((-1, 2)).mean(axis=1)
        
        # Add small random bit drops (very subtle)
        np.random.seed(42)  # consistent seed per voice
        glitch_mask = np.random.random(len(samples)) < (amount * 0.01)
        samples[glitch_mask] = samples[glitch_mask] * 0.7
        
        # Convert back
        samples_int16 = np.int16(samples)
        audio_out = AudioSegment(
            samples_int16.tobytes(),
            frame_rate=audio.frame_rate,
            sample_width=2,
            channels=1
        )
        return audio_out
    except Exception as e:
        print(f"    [WARNING] Glitch failed: {e}")
        return audio


def simple_reverb(audio, amount=0.2):
    """Add simple reverb/echo effect."""
    delay_ms = 40
    if len(audio) <= delay_ms:
        return audio
    
    try:
        delayed = AudioSegment.silent(duration=delay_ms) + audio[:-delay_ms]
        reverb_audio = audio.overlay(delayed, position=0, gain_during_overlay=amount - 6)
        return reverb_audio
    except Exception as e:
        print(f"    [WARNING] Reverb failed: {e}")
        return audio


# -------------------------
# MAIN PROCESSING
# -------------------------
def process_and_save_phoneme(syllable_name, voice_name, params, index):
    """Process syllable with speed, pitch, and Sims effects."""
    in_path = PHONEME_FOLDER / f"{syllable_name}.wav"
    if not in_path.exists():
        print(f"[WARNING] Missing: {in_path}")
        return

    try:
        # Load WAV
        audio = AudioSegment.from_wav(str(in_path))
        
        if len(audio) == 0:
            print(f"[ERROR] Empty audio from {in_path}")
            return
        
        # Step 1: Apply speed change
        speed_factor = params["speed_factor"]
        if speed_factor != 1.0:
            try:
                audio = audio.speedup(speed_factor)
            except Exception as e:
                print(f"    [WARNING] Speedup failed: {e}")
        
        # Step 2: Apply pitch shift independently
        pitch_shift = params["pitch_shift"]
        audio = change_pitch(audio, pitch_shift)
        
        # Step 3: Trim/pad to target duration (0.1s = 100ms)
        if len(audio) > TARGET_DURATION_MS:
            audio = audio[:TARGET_DURATION_MS]
        elif len(audio) < TARGET_DURATION_MS:
            silence = AudioSegment.silent(duration=(TARGET_DURATION_MS - len(audio)))
            audio = audio + silence
        
        # Step 4: Apply volume boost
        audio = audio + params["volume_change_db"]
        
        # Step 5: Add subtle Sims-style effects
        audio = add_subtle_distortion(audio, params["distortion"])
        audio = add_subtle_glitch(audio, params["glitch"])
        
        # Step 6: Apply reverb if enabled
        if params.get("reverb", False):
            audio = simple_reverb(audio, amount=0.2)
        
        # Export
        out_path = OUTPUT_FOLDER / f"{voice_name}voice{index + 1}.wav"
        audio.export(str(out_path), format="wav")
        print(f"[OK] {out_path} ({len(audio)}ms)")
        
    except Exception as e:
        print(f"[ERROR] Processing {syllable_name}: {e}")


# -------------------------
# MAIN EXECUTION
# -------------------------
if __name__ == "__main__":
    print("="*60)
    print("SIMS-STYLE CARTOON VOICE GENERATOR")
    print("="*60)
    print()
    
    generate_base_syllables()
    
    print("\n" + "="*60)
    print("PROCESSING SYLLABLES INTO CHARACTER VOICES")
    print("="*60 + "\n")
    
    for voice_name, params in VOICES.items():
        print(f"\n--- Processing: {voice_name.upper()} ---\n")
        for i, syllable_name in enumerate(BASE_SYLLABLES.keys()):
            process_and_save_phoneme(syllable_name, voice_name, params, i)
    
    print("\n" + "="*60)
    print("COMPLETE!")
    print("="*60)
