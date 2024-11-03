#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LOGIC_FPS 30

void Clock_Init(void);

int32_t Clock_SyncTicks(void);

size_t Clock_GetDateTime(char *buffer, size_t size);

double Clock_GetHighPrecisionCounter(void);

int32_t Clock_GetFrameAdvance(void);
int32_t Clock_GetLogicalFrame(void);
int32_t Clock_GetDrawFrame(void);

extern int32_t Clock_GetCurrentFPS(void);
