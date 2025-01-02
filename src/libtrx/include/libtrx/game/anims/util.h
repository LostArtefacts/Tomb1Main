#pragma once

#define ANIM_CMD_ENVIRONMENT_BITS(C) ((C & 0xC000) >> 14)
#define ANIM_CMD_PARAM_BITS(C) (C & 0x3FFF)
