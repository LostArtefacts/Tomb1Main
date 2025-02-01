#pragma once

#include "./types.h"

void Camera_InitialiseCineFrames(int32_t num_frames);
CINE_FRAME *Camera_GetCineFrame(int32_t frame_idx);
CINE_FRAME *Camera_GetCurrentCineFrame(void);
