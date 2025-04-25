# Default Collision Box Pattern

Unless explicitly defined otherwise, collision boxes for all entities (characters, items, enemies) should follow this pattern:

- Width: 50% of sprite width (x_size/2)
- X offset: 25% of sprite width (x_size/4) to center the collision box horizontally
- Height: 2 pixels
- Y offset: At the bottom of sprite (y_size-1)

This creates a small collision area at the feet of the sprite, which is ideal for:
- Character-to-character interactions
- Item pickups
- Combat collision detection
