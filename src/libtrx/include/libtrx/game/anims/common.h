#pragma once

#include "./types.h"

void Anim_InitialiseBones(int32_t num_bones);

ANIM_BONE *Anim_GetBone(int32_t bone_idx);

bool Anim_TestAbsFrameEqual(int16_t abs_frame, int16_t frame);
bool Anim_TestAbsFrameRange(int16_t abs_frame, int16_t start, int16_t end);
