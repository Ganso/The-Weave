#!/usr/bin/env python3
# resample_wavs.py — remuestrea los WAV de res/sfx/ al formato del driver de
# sonido activo, generando un "<nombre>_resampled.wav" junto a cada fuente.
# Los .res referencian SIEMPRE el _resampled.wav; los WAV originales quedan
# como fuente de autoría y no entran en la ROM.
#
# Tasa según el driver (rescomp.txt de SGDK):
#   XGM  → 14000 Hz  |  XGM2 → 13300 Hz (8 bit signed, mono, en ambos)
# El driver activo se lee de XGM_VERSION en src/audio/sound.h.
#
# Se salta los que ya existen y están al día (mtime); con --force regenera
# todos. Los _resampled.wav se COMMITEAN: así otro equipo puede compilar
# aunque no tenga ffmpeg instalado.
#
# Uso: python3 tools/resample_wavs.py [--force]
import os, re, shutil, subprocess, sys

script_dir = os.path.dirname(os.path.abspath(__file__))
repo_root  = os.path.abspath(os.path.join(script_dir, '..'))
SFX_DIR    = os.path.join(repo_root, 'res', 'sfx')
SOUND_H    = os.path.join(repo_root, 'src', 'audio', 'sound.h')

RATES = {1: 14000, 2: 13300}   # XGM_VERSION → Hz


def die(msg):
    print(f"resample_wavs: FATAL: {msg}", file=sys.stderr)
    sys.exit(1)


def target_rate():
    m = re.search(r'#define\s+XGM_VERSION\s+(\d+)', open(SOUND_H, encoding='utf-8').read())
    if not m: die(f"no encuentro XGM_VERSION en {SOUND_H}")
    ver = int(m.group(1))
    if ver not in RATES: die(f"XGM_VERSION {ver} sin tasa definida (añádela a RATES)")
    return ver, RATES[ver]


def main():
    force = '--force' in sys.argv[1:]
    ver, rate = target_rate()

    sources = []
    for dirpath, _, files in os.walk(SFX_DIR):
        for f in sorted(files):
            if f.endswith('.wav') and not f.endswith('_resampled.wav'):
                sources.append(os.path.join(dirpath, f))
    if not sources: die(f"no hay .wav fuente en {SFX_DIR}")

    have_ffmpeg = shutil.which('ffmpeg') is not None
    done = skipped = 0
    for src in sources:
        out = src[:-4] + '_resampled.wav'
        if not force and os.path.exists(out) and os.path.getmtime(out) >= os.path.getmtime(src):
            skipped += 1
            continue
        if not have_ffmpeg:
            die(f"falta '{os.path.relpath(out, repo_root)}' y no hay ffmpeg para generarlo")
        r = subprocess.run(
            ['ffmpeg', '-y', '-v', 'error', '-i', src,
             '-ar', str(rate), '-ac', '1', '-sample_fmt', 's16',
             '-map_metadata', '-1', out],
            capture_output=True, text=True)
        if r.returncode != 0:
            die(f"ffmpeg falló con {os.path.relpath(src, repo_root)}:\n{r.stderr.strip()}")
        done += 1
        print(f"  {os.path.relpath(src, SFX_DIR)} -> {rate} Hz mono")

    label = 'XGM2' if ver == 2 else 'XGM'
    print(f"resample_wavs: {done} generados, {skipped} al día ({label}, {rate} Hz)")


if __name__ == '__main__':
    main()
