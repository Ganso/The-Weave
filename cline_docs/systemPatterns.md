# System Patterns

## Code Style Guidelines

### Comments
- Comments should be placed on the same line as the code they describe, not on a separate line
- Use single-line comments (// style) for most documentation
- Only use multi-line comments (/* */ style) when documentation is necessarily complex
- Keep comments concise and to the point
- Example:
  ```c
  u16 player_health;    // Current health points of the player
  void init_game(void)  // Initialize game state and resources
  ```

### File Updates
- When updating files, always preserve existing content
- Add new sections without removing or modifying unrelated existing content
- Read and understand existing patterns before making additions

## Architecture Overview
The game is built as a traditional Sega Genesis/Megadrive ROM, using the SGDK (Sega Genesis Development Kit) framework.

### Core Systems

1. Scene Management
- Act-based structure with numbered scenes (act_1_scene_1, act_1_scene_2, etc.)
- Scene initialization and cleanup patterns using new_level() and end_level()
- Resource management for backgrounds, sprites, and palettes
- Scene transition handling with current_act and current_scene variables

2. Character System
- Character initialization and management (init_character)
- Movement and animation control (move_character, look_left)
- Character following mechanics (follow_active_character)
- Multiple character support (Linus, Clio, Xander, Swan)

3. Dialog System
- Text-based dialogue implementation
- Choice-based dialogue support
- Character portraits/faces during conversations
- Multi-language support (English/Spanish)

4. Magic Pattern System
- Musical pattern-based spells (PTRN_SLEEP, PTRN_ELECTRIC, etc.)
- Pattern activation and management
- Spell inventory system
- Pattern effects in combat and exploration

5. Combat System
- Turn-based combat mechanics
- Enemy initialization and management
- Pattern-based spell casting
- Multiple enemy types (3HeadMonkey, BadBobbin)

6. Resource Management
- Background tile and map loading
- Sprite and palette management
- Sound effect and music handling
- Memory optimization for 16-bit hardware

7. Input Handling
- Controller input processing
- Movement controls
- Pattern/spell activation
- Menu navigation

### Technical Constraints
- Limited RAM and ROM space (Sega Genesis hardware)
- Fixed screen resolution and color palette limitations
- Audio channel restrictions
- Processing power constraints of the 68000 CPU
