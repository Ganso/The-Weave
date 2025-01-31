# Technical Context

## Development Stack

### Core Technologies
- SGDK (Sega Genesis Development Kit)
- C programming language
- Motorola 68000 Assembly (boot code)
- Genesis VDP (Video Display Processor)
- YM2612 FM Synthesis & PSG Sound

### Development Tools
- Resource files (.res) for asset management
- Header files (.h) for system declarations
- Implementation files (.c) for core logic
- Python scripts for text processing (add_texts_comments.py)

### Asset Types
1. Graphics
- Background tiles and maps
- Character sprites
- Interface elements
- Font resources
- Palette files (.pal)

2. Audio
- VGM music files
- WAV sound effects
- FM synthesis patterns

3. Resource Files
- res_backgrounds
- res_characters
- res_enemies
- res_faces
- res_geesebumps
- res_interface
- res_intro
- res_items
- res_sound

## Project Structure
- src/ - Source code files
- res/ - Resource files and assets
  - Backgrounds/ - Background images and maps
  - Sound/ - Audio files
  - Sprites/ - Character and object sprites
- cline_docs/ - Project documentation

## Development Setup
- Genesis/Megadrive development environment
- Resource compilation pipeline
- Build system for ROM generation
- Debugging tools for Genesis hardware
- Header structure:
  - genesis.h included via globals.h
  - All other files inherit headers through global inclusion
  - No individual SDK path configuration needed

## Technical Constraints
- 16-bit architecture limitations
- Hardware sprite and tile restrictions
- Audio channel management
- Memory management for assets
- Performance optimization requirements
