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
    RT_NUMBER_OF,
} RING_TYPE;

typedef enum {
    RNG_OPENING,
    RNG_OPEN,
    RNG_CLOSING,
    RNG_MAIN2OPTION,
    RNG_MAIN2KEYS,
    RNG_KEYS2MAIN,
    RNG_OPTION2MAIN,
    RNG_SELECTING,
    RNG_SELECTED,
    RNG_DESELECTING,
    RNG_DESELECT,
    RNG_CLOSING_ITEM,
    RNG_EXITING_INVENTORY,
    RNG_DONE,
    RNG_FADING_OUT,
} RING_STATUS;
