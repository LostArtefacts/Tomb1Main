#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t initial;
    int32_t target;
    int32_t debuff;
    int32_t duration;

    bool is_active;
    struct {
        int32_t frame;
        int32_t value;
    } current;
} FADER;

void Fader_Init(
    FADER *fader, int32_t initial, int32_t target, int32_t frames,
    int32_t debuff);
void Fader_InitBlackToTransparent(FADER *fader, int32_t frames);
void Fader_InitTransparentToBlack(FADER *fader, int32_t frames);
void Fader_InitAnyToBlack(FADER *fader, int32_t frames);
bool Fader_IsActive(const FADER *fader);
bool Fader_Control(FADER *fader);
