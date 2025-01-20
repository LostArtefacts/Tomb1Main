#pragma once

#include "global/types_decomp.h"

#include <libtrx/game/collision.h>
#include <libtrx/game/effects.h>
#include <libtrx/game/game_flow/types.h>
#include <libtrx/game/lara/types.h>
#include <libtrx/game/lot.h>
#include <libtrx/game/objects/common.h>
#include <libtrx/game/sound/enum.h>

typedef struct {
    uint16_t draw_type;
    uint16_t tex_page;
    PHD_UV uv[4];
    PHD_UV uv_backup[4];
} OBJECT_TEXTURE;
