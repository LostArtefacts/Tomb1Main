#pragma once

// clang-format off
typedef enum {
    SPM_NORMAL     = 0,
    SPM_UNDERWATER = 1,
    SPM_ALWAYS     = 2,
#if TR_VERSION == 2
    SPM_PITCH      = 4,
#endif
} SOUND_PLAY_MODE;

typedef enum {
    UMM_FULL             = 0,
    UMM_QUIET            = 1,
    UMM_FULL_NO_AMBIENT  = 2,
    UMM_QUIET_NO_AMBIENT = 3,
    UMM_NONE             = 4,
} UNDERWATER_MUSIC_MODE;
// clang-format on
