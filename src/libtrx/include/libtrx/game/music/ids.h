#pragma once

typedef enum {
    MX_INACTIVE = -1,
#define MUSIC_TRACK_DEFINE(name, id) name = id,
#include "./ids.def"
#undef MUSIC_TRACK_DEFINE
    MX_NUMBER_OF,
} MUSIC_TRACK_ID;
