#pragma once

typedef enum {
    GFS_DISPLAY_PICTURE,
    GFS_LOAD_LEVEL,
    GFS_PLAY_LEVEL,
    GFS_PLAY_FMV,
    GFS_LEVEL_STATS,
    GFS_TOTAL_STATS,
    GFS_LOADING_SCREEN,
    GFS_EXIT_TO_TITLE,
    GFS_EXIT_TO_LEVEL,
    GFS_EXIT_TO_CINE,
    GFS_SET_CAMERA_ANGLE,
    GFS_FLIP_MAP,
    GFS_ADD_ITEM,
    GFS_REMOVE_WEAPONS,
    GFS_REMOVE_AMMO,
    GFS_REMOVE_MEDIPACKS,
    GFS_REMOVE_SCIONS,
    GFS_PLAY_SYNCED_AUDIO,
    GFS_MESH_SWAP,
    GFS_SETUP_BACON_LARA,
    GFS_LEGACY,
} GAME_FLOW_SEQUENCE_EVENT_TYPE;