#pragma once

typedef enum {
    INV_GAME_MODE = 0,
    INV_TITLE_MODE = 1,
    INV_KEYS_MODE = 2,
    INV_SAVE_MODE = 3,
    INV_LOAD_MODE = 4,
    INV_DEATH_MODE = 5,
#if TR_VERSION == 1
    INV_SAVE_CRYSTAL_MODE = 6,
#endif
} INVENTORY_MODE;

typedef enum {
    RT_MAIN = 0,
    RT_OPTION = 1,
    RT_KEYS = 2,
} RING_TYPE;

typedef enum {
    RNG_OPENING = 0,
    RNG_OPEN = 1,
    RNG_CLOSING = 2,
    RNG_MAIN2OPTION = 3,
    RNG_MAIN2KEYS = 4,
    RNG_KEYS2MAIN = 5,
    RNG_OPTION2MAIN = 6,
    RNG_SELECTING = 7,
    RNG_SELECTED = 8,
    RNG_DESELECTING = 9,
    RNG_DESELECT = 10,
    RNG_CLOSING_ITEM = 11,
    RNG_EXITING_INVENTORY = 12,
    RNG_DONE = 13,
} RING_STATUS;
