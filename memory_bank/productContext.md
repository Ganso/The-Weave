# Product Context

## Project Purpose
The Weave is a fan game of LucasArts' Loom, being developed for the Sega Genesis/Megadrive console. It aims to recreate the magical musical gameplay mechanics of Loom while telling its own unique story.

## Problems Solved
- Creates a new adventure game experience for the Sega Genesis/Megadrive
- Provides a spiritual successor/homage to the classic game Loom
- Implements a unique musical pattern-based magic system
- Delivers a story-rich experience with character interactions and choices

## Core Functionality
- Musical pattern-based magic system where players learn and cast spells
- Character dialogue system with choice mechanics
- Combat system incorporating the musical magic patterns
- Scene-based progression through different acts and locations
- Character movement and interaction with environment objects
- Resource management for sprites, backgrounds, and sound effects

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

## Asset Types
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

## Project Documentation
- `projectBrief.md`: Contains a high-level overview of the project goals, key features, and current status.
