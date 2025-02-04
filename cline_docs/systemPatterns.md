# System Patterns

## Code Organization

- All source files are in the `src/` directory
- All resources (sprites, backgrounds, etc.) are in the `res/` directory
- Only include globals.h in source files - it already includes everything needed for the project

## Resource Management

- Resources are defined in .res files in the res/ directory
- Each resource type has its own .res file (characters, faces, interface, etc.)
- Resource names follow the pattern:
  - Sprites: [category]_[name]_sprite
  - Images: [category]_[name]_image
  - Palettes: [category]_pal

## Memory Management

- Faces are loaded dynamically when needed and released after use
- Sprites are managed through the SGDK sprite engine (SPR_)
- Text is drawn directly to the background plane

## UI Patterns

- Dialog boxes use lines 23-25 of the window plane
- Text is centered based on face position and text length
- Faces can be positioned on left or right side
- Button A sprite is used for interaction prompts
- Magic animation sprite (96x8) is used for menu selection highlights

## Input Handling

- Button A is used for confirmation/advance
- Up/Down for menu navigation
- Input checking includes debouncing (prev_joy_state)

## Animation

- Character animations are state-based (idle, walking)
- Text appears character by character
- Menu selection uses animated magic effect

## Error Handling

- Memory allocation failures are checked for text encoding
- Input validation for array bounds
- State validation for character animations
