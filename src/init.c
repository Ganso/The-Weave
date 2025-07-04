#include "globals.h"

void initialize(bool first_time)    // Initialize system hardware, sprites, controllers and global game state
{
    dprintf(2,"Initializing system, first_time=%d\n", first_time);
    u8 i;

    // Initialize VPD
    VDP_init();

    // Initialize audio driver
    Z80_init();
    if (XGM_VERSION==2) {
        Z80_loadDriver(Z80_DRIVER_XGM2, 1);
        dprintf(2,"XGM2 driver loaded\n");
    }
    else {
        Z80_loadDriver(Z80_DRIVER_XGM, 1);
        dprintf(2,"XGM driver loaded\n");
    }

    // Initialize sprite Engine
    SPR_init();

    // Initialize controllers
    JOY_init();

    // Screen definitions
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    // Detect refresh rate
    u8 vers = *(u8 *)0xA10001;
    if(vers & (1 << 6)) SCREEN_FPS=50; // PAL
    else SCREEN_FPS=60; // NTSC

    // Load font and set text palette
    VDP_loadFont(font.tileset, DMA);
    VDP_setTextPalette(PAL2);

    // Initialize globals

    //  Plane A scrolls up to line 22 (176px)
    VDP_setWindowVPos(TRUE, 22);

    // Initialize palettes
    dprintf(2,"Loading palettes (initialize)\n");
    // PAL0 is the background palette. It's initialized with the background
    PAL_setPalette(PAL1, characters_pal.data, DMA); // Characters palette
    PAL_setPalette(PAL2, interface_pal.data, DMA); // Interface palette
    // PAL2 is the enemies palette. It's initialized with the enemies

    // Interface: Face backgrounds
    dprintf(2,"Loading face backgrounds\n");
    spr_face_left = SPR_addSpriteSafe ( &face_left_sprite, 0, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_left, HIDDEN);
    spr_face_right = SPR_addSpriteSafe ( &face_right_sprite, 256, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_right, HIDDEN);
    SPR_setDepth (spr_face_left, SPR_MIN_DEPTH+1); // Face background are above anything but faces
    SPR_setDepth (spr_face_right, SPR_MIN_DEPTH+1); // Face background are above anything but faces

    // Interface: Button A
    spr_int_button_A = SPR_addSpriteSafe (&int_button_A_sprite, 0, 0, TILE_ATTR(PAL2, false, false, false));
    SPR_setVisibility (spr_int_button_A, HIDDEN);

    // Patterns & combat context
    if (first_time) {
        dprintf(2,"Initializing patterns\n");
        init_player_patterns();
        dprintf(2,"Initializing enemy classes\n");
        init_enemy_classes();
    }
    dprintf(2,"Initializing combat context\n");
    combat_state          = COMBAT_NO;
    combatContext.frameInState   = 0;
    combatContext.activePattern  = PATTERN_PLAYER_NONE;
    combatContext.effectTimer    = 0;
    combatContext.patternReversed= FALSE;
    combatContext.noteTimer      = 0;
    combatContext.playerNotes    = 0;
    combatContext.activeEnemy    = ENEMY_NONE;

    // Items
    last_interacted_item=ITEM_NONE;

    // Release active character, faces, enemies and items
    for (i=0;i<MAX_CHR;i++) {
        if (obj_character[i].active==true) release_character(i);
    }
    for (i=0;i<MAX_FACE;i++) {
        if (obj_face[i].active==true) release_face(i);
    }
    for (i=0;i<MAX_ENEMIES;i++) {
        if (obj_enemy[i].obj_character.active==true) release_enemy(i);
    }
    for (i=0;i<MAX_ITEMS;i++) {
        if (obj_item[i].entity.active==true) release_item(i);
    }
}

// initialize level and load background
void new_level(const TileSet *tile_bg, const MapDefinition *map_bg, const TileSet *tile_front, const MapDefinition *map_front, Palette new_pal, u16 new_background_width, u8 new_scroll_mode, u8 new_scroll_speed)    // Load and setup a new game level with background layers and scroll settings
{
    dprintf(2,"Loading new level: bg_width=%d scroll_mode=%d\n", new_background_width, new_scroll_mode);
    
    initialize(false); // Reset hardware when starting each level, but don't change only first-time options
    
    // Reset tile index to start fresh
    tile_ind = TILE_USER_INDEX;
    
    // Tile_bg and Map_bg are the background layer. They can be NULL
    if ((tile_bg!=NULL) && (map_bg!=NULL)) {
        dprintf(2,"Loading background tileset, tiles=%d\n", tile_bg->numTile);
        VDP_loadTileSet(tile_bg, tile_ind, CPU);
        background_BGB = MAP_create(map_bg, BG_B, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
        tile_ind += tile_bg->numTile;
    }
    else background_BGB=NULL;

    // Tile_front and Map_front are the foreground layer. Thay can't be NULL.
    dprintf(2,"Loading foreground tileset, tiles=%d\n", tile_front->numTile);
    VDP_loadTileSet(tile_front, tile_ind, CPU);
    background_BGA = MAP_create(map_front, BG_A, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += tile_front->numTile;

    // Set palettes after loading all tiles to avoid flicker
    dprintf(2,"Loading palettes (new level)\n");
    PAL_setPalette(PAL0, new_pal.data, DMA);
    PAL_setPalette(PAL1, characters_pal.data, DMA);
    PAL_setPalette(PAL2, interface_pal.data, DMA);

    background_scroll_mode=new_scroll_mode;
    scroll_speed=new_scroll_speed;
    background_width=new_background_width;

    offset_BGA = FASTFIX32_FROM_INT(0);
    offset_BGB=0;

    if (background_scroll_mode==BG_SCRL_USER_LEFT) { // We should start at the rightmost edge of the screen
        offset_BGA = FASTFIX32_FROM_INT(background_width - SCREEN_WIDTH);
    }

    interface_active=false; // No interface by default
    player_scroll_active=false; // You can't scroll the screen by default
    movement_active=false; // You can't move by default
    player_has_paused=false; // Player has not paused the game yet

    dprintf(2,"everthing initialized, background scroll mode=%d, scroll speed=%d\n", background_scroll_mode, scroll_speed);
    update_bg(false);
}

// Free all resources used by the level
void end_level() {    // Clean up level resources and reset game state
    dprintf(2,"Ending level, freeing resources\n");
    
    // Fade out music and screen
    fade_music(SCREEN_FPS);
    PAL_fadeOutAll(SCREEN_FPS,false);

    // Free background maps
    if (background_BGA) {
        MAP_release(background_BGA);
        background_BGA = NULL;
    }
    if (background_BGB) {
        MAP_release(background_BGB);
        background_BGB = NULL;
    }

    // Release active characters
    dprintf(2,"Releasing active characters\n");
    for (u16 i = 0; i < MAX_CHR; i++) {
        if (obj_character[i].active) {
            release_character(i);
        }
    }

    // Release active faces
    for (u16 i = 0; i < MAX_FACE; i++) {
        if (obj_face[i].active) {
            release_face(i);
        }
    }

    // Release active enemies
    for (u16 i = 0; i < MAX_ENEMIES; i++) {
        if (obj_enemy[i].obj_character.active) {
            release_enemy(i);
        }
    }

    // Release active items
    for (u16 i = 0; i < MAX_ITEMS; i++) {
        if (obj_item[i].entity.active) {
            release_item(i);
        }
    }

    // Reset combat context
    combatContext = (CombatContext){
        .frameInState    = 0,
        .activePattern   = PATTERN_PLAYER_NONE,
        .effectTimer     = 0,
        .patternReversed = FALSE,
        .noteTimer       = 0,
        .playerNotes     = 0,
        .activeEnemy     = ENEMY_NONE
    };
    combat_state = COMBAT_NO;

    // player_patterns_enabled = false; // Mantener habilitado para permitir lanzar hechizos

    last_interacted_item = ITEM_NONE;

    // Reset scroll values
    offset_BGA = FASTFIX32_FROM_INT(0);
    offset_BGB = 0;
    background_scroll_mode = 0;
    scroll_speed = 0;
    player_scroll_active = FALSE;
    movement_active = FALSE;

    // Reset screen limits
    x_limit_min = 0;
    x_limit_max = 0;
    y_limit_min = 0;
    y_limit_max = 0;

    // Reset game state
    //active_character = CHR_NONE; <-- Don't reset: The Act level code should do it if needed
    interface_active = FALSE;

    // Reset all sprites after releasing everything
    VDP_releaseAllSprites();
    SPR_reset();
    
    // Reset VDP state
    tile_ind = TILE_USER_INDEX;
    VDP_resetScreen();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x000000));
    VDP_setWindowVPos(TRUE, 22);

    dprintf(2,"Level cleanup complete\n");
}
