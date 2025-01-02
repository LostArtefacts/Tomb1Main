#pragma once

// clang-format off
typedef enum {
    AC_NULL          = 0,
    AC_MOVE_ORIGIN   = 1,
    AC_JUMP_VELOCITY = 2,
    AC_ATTACK_READY  = 3,
    AC_DEACTIVATE    = 4,
    AC_SOUND_FX      = 5,
    AC_EFFECT        = 6,
} ANIM_COMMAND;

typedef enum {
    ACE_ALL   = 0,
    ACE_LAND  = 1,
    ACE_WATER = 2,
} ANIM_COMMAND_ENVIRONMENT;

typedef enum {
    BF_MATRIX_POP  = 1,
    BF_MATRIX_PUSH = 2,
    BF_ROT_X       = 4,
    BF_ROT_Y       = 8,
    BF_ROT_Z       = 16,
} BONE_FLAGS;
// clang-format on
