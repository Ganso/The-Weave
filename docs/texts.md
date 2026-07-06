# Textos y choices — guía de autoría

## Diálogos: data/texts.csv

Columnas: `set,id,face,side,time,es,en`

- **set**: agrupa por escena, con la filosofía de nombres del proyecto:
  `system_dialog`, `act1_bedroom`, `act1_corridor`, `act1_hall`, `act1_forest`...
  El generador emite el define `ACT1_BEDROOM` (índice en `dialogs[]`) y el enum
  de ids.
- **id**: `A<n>_<ESCENA>_<QUE_DICE>` (p.ej. `A1_BEDROOM_SLEPT_BAD`) o `SYSMSG_*`
  para sistema. Filas con id `NULL` = terminador de cluster (el generador crea
  `A1_BEDROOM_TERM_n` y cierra el grupo de `talk_cluster`).
- **face**: `FACE_linus`, `FACE_clio`, `FACE_xander`, `FACE_swan`, `FACE_none`.
- **side**: `SIDE_LEFT` / `SIDE_RIGHT` / `SIDE_NONE` (posiciona como LEFT).
- **time**: segundos máximos (`DEFAULT_TALK_TIME` = 10) antes de auto-avanzar.
- **es / en**: el texto. Escapes: `|` = salto de línea; `@[texto@]` = color
  destacado. Máx ~30 caracteres visibles por línea.

Caracteres españoles: se escriben NORMALES en el CSV (ñ, á, ¿...) —
`encode_spanish_text` los convierte al charset de la fuente en runtime.

Tras editar: compilar (el build ejecuta `gen_texts.py`) o `python3 tools/gen_texts.py`.
Salida: `src/narrative/texts_data.{c,h}` (NO editar).

Uso: en una escena, `say SET ID [sound]`; en un hook C,
`talk_dialog(&dialogs[SET][ID], sound)` o `talk_cluster(...)` (encadena hasta TERM).

## Clusters

Un `talk_cluster`/`say_cluster` muestra consecutivamente todas las filas desde el
id inicial hasta el siguiente terminador `NULL`. Para partir un grupo, inserta una
fila `NULL` entre medias.

## Choices: data/choices.csv

Columnas: `set,item,face,side,time,es_1..es_4,en_1..en_4`

- **set**: mismo criterio de nombres (`act1_hall_choice` → `ACT1_HALL_CHOICE`).
- **item**: índice del choice dentro del set (0, 1, ...), consecutivos.
- Opciones vacías = no existen (el nº de opciones se calcula; ES y EN deben coincidir).

Salida: `src/narrative/choices_data.{c,h}` (gen_choices.py; NO editar).

Uso en escena: `choice ACT1_HALL_CHOICE 0` deja la opción elegida (0-based) en
`last_choice`; después `branch n goto <label>` para ramificar, o
`say_response SET BASE_ID` si las respuestas son textos correlativos
(muestra `BASE_ID + last_choice`).

## Voces (gibberish)

Las voces animalese de los diálogos se generan con
`tools/voice/generate_animalese_voices.py` (venv con librosa/numpy/soundfile);
salida en `res/Sound/Dialogs/`. Perfiles: woman, man, deep. La fuente de fonemas
es animalese.wav del proyecto animalese.js (ver Acknowledgements del README).
