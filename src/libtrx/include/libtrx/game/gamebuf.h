#pragma once

#include <stdint.h>

typedef enum {
// clang-format off
#if TR_VERSION == 1
    GBUF_TEXTURE_PAGES            = 0,
    GBUF_MESH_POINTERS            = 1,
    GBUF_MESHES                   = 2,
    GBUF_ANIMS                    = 3,
    GBUF_ANIM_CHANGES             = 4,
    GBUF_ANIM_RANGES              = 5,
    GBUF_ANIM_COMMANDS            = 6,
    GBUF_ANIM_BONES               = 7,
    GBUF_ANIM_FRAMES              = 8,
    GBUF_ROOMS                    = 9,
    GBUF_ROOM_MESH                = 10,
    GBUF_ROOM_DOOR                = 11,
    GBUF_ROOM_SECTOR              = 12,
    GBUF_ROOM_LIGHTS              = 13,
    GBUF_ROOM_STATIC_MESHES       = 14,
    GBUF_FLOOR_DATA               = 15,
    GBUF_ITEMS                    = 16,
    GBUF_CAMERAS                  = 17,
    GBUF_SOUND_FX                 = 18,
    GBUF_BOXES                    = 19,
    GBUF_OVERLAPS                 = 20,
    GBUF_GROUNDZONE               = 21,
    GBUF_FLYZONE                  = 22,
    GBUF_ANIMATING_TEXTURE_RANGES = 23,
    GBUF_CINEMATIC_FRAMES         = 24,
    GBUF_LOADDEMO_BUFFER          = 25,
    GBUF_EXTRA_DOOR_STUFF         = 26,
    GBUF_EFFECTS                  = 27,
    GBUF_CREATURE_INFO            = 28,
    GBUF_CREATURE_LOT             = 29,
    GBUF_SAMPLE_INFOS             = 30,
    GBUF_SAMPLES                  = 31,
    GBUF_TRAP_DATA                = 32,
    GBUF_CREATURE_DATA            = 33,
    GBUF_VERTEX_BUFFER            = 34,
    GBUF_NUM_MALLOC_TYPES         = 35,
#elif TR_VERSION == 2
    GBUF_TEMP_ALLOC               = 0,
    GBUF_TEXTURE_PAGES            = 1,
    GBUF_MESH_POINTERS            = 2,
    GBUF_MESHES                   = 3,
    GBUF_ANIMS                    = 4,
    GBUF_STRUCTS                  = 5,
    GBUF_ANIM_RANGES              = 6,
    GBUF_ANIM_COMMANDS            = 7,
    GBUF_ANIM_BONES               = 8,
    GBUF_ANIM_FRAMES              = 9,
    GBUF_ROOM_TEXTURES            = 10,
    GBUF_ROOMS                    = 11,
    GBUF_ROOM_MESH                = 12,
    GBUF_ROOM_PORTALS             = 13,
    GBUF_ROOM_FLOOR               = 14,
    GBUF_ROOM_LIGHTS              = 15,
    GBUF_ROOM_STATIC_MESHES       = 16,
    GBUF_FLOOR_DATA               = 17,
    GBUF_ITEMS                    = 18,
    GBUF_CAMERAS                  = 19,
    GBUF_SOUND_FX                 = 20,
    GBUF_BOXES                    = 21,
    GBUF_OVERLAPS                 = 22,
    GBUF_GROUND_ZONE              = 23,
    GBUF_FLY_ZONE                 = 24,
    GBUF_ANIMATING_TEXTURE_RANGES = 25,
    GBUF_CINEMATIC_FRAMES         = 26,
    GBUF_LOAD_DEMO_BUFFER         = 27,
    GBUF_SAVE_DEMO_BUFFER         = 28,
    GBUF_CINEMATIC_EFFECTS        = 29,
    GBUF_MUMMY_HEAD_TURN          = 30,
    GBUF_EXTRA_DOOR_STUFF         = 31,
    GBUF_EFFECTS_ARRAY            = 32,
    GBUF_CREATURE_DATA            = 33,
    GBUF_CREATURE_LOT             = 34,
    GBUF_SAMPLE_INFOS             = 35,
    GBUF_SAMPLES                  = 36,
    GBUF_SAMPLE_OFFSETS           = 37,
    GBUF_ROLLING_BALL_STUFF       = 38,
    GBUF_SKIDOO_STUFF             = 39,
    GBUF_LOAD_PICTURE_BUFFER      = 40,
    GBUF_FMV_BUFFERS              = 41,
    GBUF_POLYGON_BUFFERS          = 42,
    GBUF_ORDER_TABLES             = 43,
    GBUF_CLUTS                    = 44,
    GBUF_TEXTURE_INFOS            = 45,
    GBUF_SPRITE_INFOS             = 46,
    GBUF_NUM_MALLOC_TYPES         = 47,
#endif
    // clang-format on
} GAME_BUFFER;

extern void *GameBuf_Alloc(size_t alloc_size, GAME_BUFFER buffer);
