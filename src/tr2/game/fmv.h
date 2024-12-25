#pragma once

#include <stdbool.h>

bool FMV_Play(const char *file_name);
bool FMV_PlayIntro(const char *file_name_1, const char *file_name_2);
bool FMV_IsPlaying(void);
