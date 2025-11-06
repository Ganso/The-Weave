#!/usr/bin/env python3
# ============================================================
# ANIMALESE PHONEME GENERATOR FOR MEGADRIVE
# 
# RAW = Exactamente como viene del original, SIN normalización
# Fricativas se copian directamente del RAW (sin procesar)
# ============================================================

import urllib.request
from pathlib import Path
import numpy as np
import struct
import librosa
import soundfile as sf

# ============================================================
# CONSTANTS
# ============================================================

SAMPLE_FREQ = 44100
LIBRARY_LETTER_SECS = 0.15
LIBRARY_SAMPLES_PER_LETTER = int(LIBRARY_LETTER_SECS * SAMPLE_FREQ)
OUTPUT_LETTER_SECS = 0.075
OUTPUT_SAMPLES_PER_LETTER = int(OUTPUT_LETTER_SECS * SAMPLE_FREQ)

DOWNLOAD_DIR = Path("animalese_download")
PHONEMES_DIR = Path("phonemes_animalese")
SYNTHESIS_DIR = Path("synthesis_animalese")

ANIMALESE_WAV_URL = "https://github.com/Acedio/animalese.js/raw/refs/heads/master/animalese.wav"

# Fricativas - se copian directamente del RAW
FRICATIVES = set("STVXZF")

VOICE_TYPES = {
    "woman": {
        "pitch_semitones": +3,
        "volume_db": 6,
        "lowpass_cutoff": 3200,
        "distortion": 0.02,
        "glitch": 0.03,
        "reverb_decay": 0.0,
    },
    "man": {
        "pitch_semitones": -2,
        "volume_db": 6,
        "lowpass_cutoff": 3500,
        "distortion": 0.03,
        "glitch": 0.04,
        "reverb_decay": 0.0,
    },
    "deep": {
        "pitch_semitones": -5,
        "volume_db": 3,
        "lowpass_cutoff": 2800,
        "distortion": 0.01,
        "glitch": 0.02,
        "reverb_decay": 0.4,
    },
}

TEST_SENTENCES = [
    {
        "id": "test_1_overslept",
        "es": "creo que he dormido demasiado",
        "en": "i think ive overslept",
    },
    {
        "id": "test_2_mother",
        "es": "perdoname maestro",
        "en": "forgive me master",
    },
    {
        "id": "test_3_island",
        "es": "la isla del gremio de los tejedores",
        "en": "the weavers guild island",
    },
    {
        "id": "test_4_weave",
        "es": "esto es una prueba de sonido animalés para el juego the weave",
        "en": "this is a sound test animalese for the game the weave",
    },
]

# ============================================================
# STEP 1: DESCARGAR ANIMALESE.WAV
# ============================================================

def download_animalese_wav():
    DOWNLOAD_DIR.mkdir(exist_ok=True)
    wav_path = DOWNLOAD_DIR / "animalese.wav"
    
    if wav_path.exists():
        print(f"[SKIP] {wav_path} ya existe")
        return wav_path
    
    print(f"[DL] Descargando animalese.wav desde GitHub...")
    print(f"     URL: {ANIMALESE_WAV_URL}\n")
    
    try:
        def reporthook(blocknum, blocksize, totalsize):
            downloaded = blocknum * blocksize
            if totalsize > 0:
                percent = min(100, int(downloaded * 100 / totalsize))
                print(f"     {percent}%", end='\r')
        
        urllib.request.urlretrieve(ANIMALESE_WAV_URL, str(wav_path), reporthook)
        print(f"\n✓ Descargado a {wav_path}\n")
        return wav_path
    except Exception as e:
        print(f"[ERROR] Descarga falló: {e}")
        return None

# ============================================================
# STEP 2: PARSEA WAV RIFF
# ============================================================

def parse_wav_riff(wav_file):
    print(f"[PARSE] Parseando WAV RIFF desde {wav_file.name}...\n")
    
    with open(wav_file, 'rb') as f:
        riff_header = f.read(4)
        if riff_header != b'RIFF':
            print(f"[ERROR] No es archivo RIFF válido")
            return None, None
        
        file_size = struct.unpack('<I', f.read(4))[0]
        wave_header = f.read(4)
        if wave_header != b'WAVE':
            print(f"[ERROR] No es archivo WAVE válido")
            return None, None
        
        fmt_data = None
        audio_data = None
        
        while True:
            chunk_id = f.read(4)
            if len(chunk_id) < 4:
                break
            
            chunk_size = struct.unpack('<I', f.read(4))[0]
            
            if chunk_id == b'fmt ':
                fmt_data = f.read(chunk_size)
                print(f"     ✓ Found fmt chunk ({chunk_size} bytes)")
            elif chunk_id == b'data':
                audio_data = f.read(chunk_size)
                print(f"     ✓ Found data chunk ({chunk_size} bytes)\n")
                break
            else:
                f.read(chunk_size)
        
        if fmt_data is None or audio_data is None:
            print(f"[ERROR] WAV incompleto")
            return None, None
        
        audio_format, num_channels, sample_rate, byte_rate, block_align, bits_per_sample = \
            struct.unpack('<HHIIHH', fmt_data[:16])
        
        print(f"     Format: {audio_format}")
        print(f"     Channels: {num_channels}")
        print(f"     Sample rate: {sample_rate} Hz")
        print(f"     Bits per sample: {bits_per_sample}")
        print(f"     Total audio bytes: {len(audio_data)}\n")
        
        if bits_per_sample == 8 and num_channels == 1:
            samples = np.frombuffer(audio_data, dtype=np.uint8).astype(np.float32)
            samples = (samples - 128) / 128.0
            print(f"     Convertido: 8-bit unsigned a float32 [-1, 1]\n")
        elif bits_per_sample == 16 and num_channels == 1:
            samples = np.frombuffer(audio_data, dtype=np.int16).astype(np.float32) / 32768.0
            print(f"     Convertido: 16-bit signed a float32 [-1, 1]\n")
        else:
            print(f"[ERROR] Formato no soportado")
            return None, None
        
        print(f"     Total samples: {len(samples)}")
        return samples, sample_rate

# ============================================================
# STEP 3: EXTRAE 26 LETRAS DEL ORIGINAL
# ============================================================

def extract_letters(samples, sr):
    print(f"[EXTRACT] Extrayendo 26 letras del original...\n")
    
    letters = {}
    for letter_idx, letter in enumerate("ABCDEFGHIJKLMNOPQRSTUVWXYZ"):
        start_sample = letter_idx * LIBRARY_SAMPLES_PER_LETTER
        end_sample = start_sample + LIBRARY_SAMPLES_PER_LETTER
        
        if end_sample <= len(samples):
            segment = samples[start_sample:end_sample]
            letters[letter] = segment
            print(f"  ✓ {letter}: {len(segment):5d} muestras")
    
    print()
    return letters

# ============================================================
# STEP 4: GENERA FONEMAS PARA CADA VOZ
# ============================================================

def pitch_shift_librosa(audio, sr, semitones):
    if semitones == 0:
        return audio
    try:
        return librosa.effects.pitch_shift(audio, sr=sr, n_steps=semitones)
    except Exception:
        return audio

def apply_simple_reverb(audio, decay=0.3, delay_ms=50):
    if decay <= 0:
        return audio
    
    delay_samples = int(delay_ms * SAMPLE_FREQ / 1000)
    decay_factor = 1 - decay
    padded = np.zeros(len(audio) + delay_samples)
    padded[:len(audio)] = audio
    
    for i in range(len(audio)):
        if i + delay_samples < len(padded):
            padded[i + delay_samples] += audio[i] * decay_factor
    
    return padded[:len(audio)]

def add_distortion(audio, amount):
    if amount <= 0:
        return audio
    audio_f = audio.astype(np.float32)
    distorted = np.tanh(audio_f * (1 + amount * 3)) * 0.85
    blended = audio_f * (1 - amount * 0.5) + distorted * (amount * 0.5)
    return blended

def add_glitch(audio, amount):
    if amount <= 0:
        return audio
    audio_f = audio.astype(np.float32).copy()
    np.random.seed(42)
    mask = np.random.random(len(audio)) < (amount * 0.02)
    audio_f[mask] *= 0.7
    return audio_f

def normalize_audio(audio):
    """Normaliza audio sin cambiar el rango dinámico significativamente"""
    peak = np.max(np.abs(audio))
    if peak > 1.0:
        # Solo normaliza si está clipped
        return audio / peak * 0.99
    return audio

def generate_phoneme(audio, sr, params):
    """Genera UN fonema procesado (para no-fricativas)"""
    
    audio = pitch_shift_librosa(audio, sr, params["pitch_semitones"])
    audio = add_distortion(audio, params["distortion"])
    audio = add_glitch(audio, params["glitch"])
    
    if params["reverb_decay"] > 0:
        audio = apply_simple_reverb(audio, decay=params["reverb_decay"], delay_ms=40)
    
    db_factor = 10 ** (params["volume_db"] / 20)
    audio = audio * db_factor
    audio = normalize_audio(audio)
    
    return audio

def generate_all_phonemes(raw_letters, sr):
    """
    Genera todos los fonemas.
    RAW: SIN PROCESAR, exactamente como viene del original
    FRICATIVAS: Se copian directamente del RAW
    OTRAS: Se procesan normalmente
    """
    
    phonemes = {"raw": {}}
    
    # RAW: Copiar EXACTAMENTE como viene, SIN normalización
    print("[GEN] Generando fonemas RAW...\n")
    for letter, audio in raw_letters.items():
        # CRÍTICO: Sin normalize_audio() para RAW
        phonemes["raw"][letter] = audio.astype(np.float32)
    print("✓ RAW completo (sin procesar)\n")
    
    # Voces modificadas
    for voice_name, params in VOICE_TYPES.items():
        print(f"[GEN] Generando fonemas {voice_name.upper()}...\n")
        phonemes[voice_name] = {}
        
        for letter, audio in raw_letters.items():
            if letter in FRICATIVES:
                # FRICATIVAS: Copiar exactamente del RAW sin modificar
                phonemes[voice_name][letter] = audio.astype(np.float32)
            else:
                # OTRAS: Procesar normalmente
                phonemes[voice_name][letter] = generate_phoneme(audio, sr, params)
        
        print(f"✓ {voice_name.upper()} completo (fricativas copiadas del RAW)\n")
    
    return phonemes

# ============================================================
# STEP 5: GUARDA FONEMAS EN DISCO (FINALES)
# ============================================================

def save_all_phonemes(phonemes, sr):
    """Guarda TODOS los fonemas generados"""
    
    print("[SAVE] Guardando fonemas generados...\n")
    
    for voice_name in phonemes.keys():
        voice_dir = PHONEMES_DIR / voice_name
        voice_dir.mkdir(parents=True, exist_ok=True)
        
        for letter, audio in phonemes[voice_name].items():
            out_path = voice_dir / f"{letter}.wav"
            sf.write(str(out_path), audio, sr)
        
        print(f"✓ {voice_name.upper()}: 26 fonemas guardados en {voice_dir}/")
    
    print()

# ============================================================
# STEP 6: PRUEBA DE SÍNTESIS (SOLO LECTURA DE FONEMAS)
# ============================================================

def text_to_letters(text):
    """Convierte texto a secuencia de letras"""
    letters = []
    for char in text.upper():
        if 'A' <= char <= 'Z':
            letters.append(char)
    return letters

def synthesize_from_phonemes(letter_seq, phonemes, voice_name):
    """Sintetiza una frase LEYENDO los fonemas guardados"""
    
    frames = []
    bank = phonemes[voice_name]
    
    for letter in letter_seq:
        if letter not in bank:
            continue
        
        full_audio = bank[letter].astype(np.float32)
        interpolated = np.zeros(OUTPUT_SAMPLES_PER_LETTER, dtype=np.float32)
        
        for i in range(OUTPUT_SAMPLES_PER_LETTER):
            source_idx = int(i * 1.0)  # pitch = 1.0
            if source_idx >= len(full_audio):
                source_idx = len(full_audio) - 1
            interpolated[i] = full_audio[source_idx]
        
        frames.append(interpolated)
    
    if not frames:
        return np.array([], dtype=np.float32)
    
    return np.concatenate(frames)

# ============================================================
# MAIN PIPELINE
# ============================================================

def main():
    print("\n" + "="*70)
    print("ANIMALESE PHONEME GENERATOR FOR MEGADRIVE")
    print("="*70)
    print("\nOBJETIVO: Generar 26 fonemas × 4 voces")
    print("RAW: Exactamente como viene del original, SIN PROCESAR")
    print("FRICATIVAS: Copiadas directamente del RAW a cada voz")
    print("OTRAS: Procesadas normalmente por voz\n")
    print("="*70 + "\n")
    
    PHONEMES_DIR.mkdir(exist_ok=True)
    SYNTHESIS_DIR.mkdir(exist_ok=True)
    
    # STEP 1: Descarga
    print("[1/6] Downloading animalese.wav...\n")
    wav_file = download_animalese_wav()
    if not wav_file:
        return
    
    # STEP 2: Parsea WAV RIFF
    print("[2/6] Parsing WAV RIFF...\n")
    samples, sr = parse_wav_riff(wav_file)
    if samples is None:
        return
    
    # STEP 3: Extrae letras
    print("[3/6] Extracting 26 letters...\n")
    raw_letters = extract_letters(samples, sr)
    
    # STEP 4: Genera fonemas
    print("[4/6] Generating phonemes for all voices...\n")
    phonemes = generate_all_phonemes(raw_letters, sr)
    
    # STEP 5: Guarda fonemas
    print("[5/6] Saving generated phonemes...\n")
    save_all_phonemes(phonemes, sr)
    
    # STEP 6: Pruebas de síntesis
    print("[6/6] Testing synthesis (READ-ONLY)...\n")
    
    for test in TEST_SENTENCES:
        print(f"  {test['id']}:")
        
        for lang_key in ["es", "en"]:
            text = test[lang_key]
            letter_seq = text_to_letters(text)
            
            if not letter_seq:
                print(f"    {lang_key.upper()}: vacío")
                continue
            
            for voice in ["raw", "woman", "man", "deep"]:
                audio = synthesize_from_phonemes(letter_seq, phonemes, voice)
                out_path = SYNTHESIS_DIR / f"{test['id']}_{lang_key}_{voice}.wav"
                sf.write(str(out_path), audio, sr)
                duration_ms = len(audio) / sr * 1000
                print(f"    {voice.upper()} {lang_key}: {duration_ms:6.0f}ms")
        
        print()
    
    print("="*70)
    print("✓ PHONEME GENERATION COMPLETE")
    print("="*70)
    print(f"\nFonemas guardados en: {PHONEMES_DIR}/")
    print(f"  - raw/       (26 fonemas originales, SIN PROCESAR)")
    print(f"  - woman/     (26 fonemas mujer + fricativas RAW)")
    print(f"  - man/       (26 fonemas hombre + fricativas RAW)")
    print(f"  - deep/      (26 fonemas grave + fricativas RAW)")
    print(f"\nFricativas (S,T,V,X,Z,F): Idénticas al RAW en todas las voces\n")

if __name__ == "__main__":
    main()
