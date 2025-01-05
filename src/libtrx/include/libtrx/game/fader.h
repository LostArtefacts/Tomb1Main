#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t initial;
    int32_t target;

    // This value controls how much to keep the last frame after the animation
    // is done.
    int32_t debuff;
    int32_t duration;
} FADER_ARGS;

typedef struct {
    FADER_ARGS args;
    bool is_active;
    bool has_fired;
    struct {
        int32_t frame;
        int32_t value;
    } current;
} FADER;

void Fader_Init(FADER *fader, FADER_ARGS args);
void Fader_InitEmpty(FADER *fader);
void Fader_InitBlackToTransparent(FADER *fader, int32_t frames);
void Fader_InitTransparentToBlack(FADER *fader, int32_t frames);
void Fader_InitTransparentToSemiBlack(FADER *fader, int32_t frames);
void Fader_InitAnyToBlack(FADER *fader, int32_t frames);
void Fader_InitAnyToSemiBlack(FADER *fader, int32_t frames);
bool Fader_IsActive(const FADER *fader);
void Fader_Finish(FADER *fader);
bool Fader_Control(FADER *fader);
int32_t Fader_GetCurrentValue(const FADER *fader);
