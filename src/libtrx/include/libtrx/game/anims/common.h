#pragma once

#include "./types.h"

extern ANIM *g_Anims;

void Anim_InitialiseBones(int32_t num_bones);

ANIM *Anim_GetAnim(int32_t anim_idx);
ANIM_BONE *Anim_GetBone(int32_t bone_idx);

bool Anim_TestAbsFrameEqual(int16_t abs_frame, int16_t frame);
bool Anim_TestAbsFrameRange(int16_t abs_frame, int16_t start, int16_t end);
