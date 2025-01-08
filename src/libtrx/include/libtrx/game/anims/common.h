#pragma once

#include "./types.h"

extern int16_t *g_AnimCommands;

void Anim_InitialiseAnims(int32_t num_anims);
void Anim_InitialiseChanges(int32_t num_changes);
void Anim_InitialiseRanges(int32_t num_ranges);
void Anim_InitialiseBones(int32_t num_bones);

ANIM *Anim_GetAnim(int32_t anim_idx);
ANIM_CHANGE *Anim_GetChange(int32_t change_idx);
ANIM_RANGE *Anim_GetRange(int32_t range_idx);
int16_t *Anim_GetCommand(int32_t cmd_idx);
ANIM_BONE *Anim_GetBone(int32_t bone_idx);

bool Anim_TestAbsFrameEqual(int16_t abs_frame, int16_t frame);
bool Anim_TestAbsFrameRange(int16_t abs_frame, int16_t start, int16_t end);
