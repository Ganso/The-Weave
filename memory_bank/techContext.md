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

## Code Conventions
- Data types defined in Sega Genesis libraries are used: u8, u16, s8, s16, etc.
- All include statements are centralized in `globals.h`.
- Other header files (.h) do not require include statements.
- C files (.c) only include `globals.h`.

## Technical Constraints
- 16-bit architecture limitations
- Hardware sprite and tile restrictions
- Audio channel management
- Memory management for assets
- Performance optimization requirements
