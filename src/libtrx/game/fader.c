#include "game/fader.h"

#include "config.h"
#include "game/clock/const.h"
#include "utils.h"

#define ANY (-1)
#define TRANSPARENT 0
#define HALF_OPAQUE 127
#define OPAQUE 255

void Fader_Init(FADER *const fader, const FADER_ARGS args)
{
    fader->args = args;

    if (args.initial == ANY) {
        if (fader->is_active && fader->has_fired) {
            fader->args.initial = fader->current.value;
        } else {
            fader->args.initial = 0;
            fader->current.frame = 0;
        }
    } else {
        fader->current.value = args.initial;
        fader->current.frame = 0;
    }

    if (g_Config.visuals.enable_fade_effects && args.duration > 0) {
        fader->is_active = true;
    } else {
        fader->is_active = false;
        fader->current.frame = args.duration + args.debuff;
        fader->current.value = args.target;
    }
}

void Fader_InitEmpty(FADER *const fader)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = TRANSPARENT,
            .target = TRANSPARENT,
            .duration = 0,
            .debuff = 0,
        });
}

void Fader_InitBlackToTransparent(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = OPAQUE,
            .target = TRANSPARENT,
            .duration = duration,
            .debuff = 0,
        });
}

void Fader_InitTransparentToBlack(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = TRANSPARENT,
            .target = OPAQUE,
            .duration = duration,
            .debuff = LOGIC_FPS / 6,
        });
}

void Fader_InitTransparentToSemiBlack(
    FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = TRANSPARENT,
            .target = HALF_OPAQUE,
            .duration = duration,
            .debuff = LOGIC_FPS / 6,
        });
}

void Fader_InitAnyToBlack(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = ANY,
            .target = OPAQUE,
            .duration = duration,
            .debuff = LOGIC_FPS / 6,
        });
}

void Fader_InitAnyToSemiBlack(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = ANY,
            .target = HALF_OPAQUE,
            .duration = duration,
            .debuff = LOGIC_FPS / 6,
        });
}

void Fader_Finish(FADER *const fader)
{
    fader->is_active = false;
    fader->current.frame = fader->args.duration + fader->args.debuff;
    fader->current.value = fader->args.target;
}

int32_t Fader_GetCurrentValue(const FADER *const fader)
{
    if (!g_Config.visuals.enable_fade_effects || fader->args.duration == 0) {
        return TRANSPARENT;
    }
    return fader->current.value;
}

bool Fader_IsActive(const FADER *const fader)
{
    if (!g_Config.visuals.enable_fade_effects) {
        return false;
    }
    return fader->is_active;
}

bool Fader_Control(FADER *const fader)
{
    if (!fader->is_active) {
        return false;
    }
    fader->has_fired = true;

    fader->current.value = fader->args.initial
        + (fader->args.target - fader->args.initial)
            * (fader->current.frame / (float)MAX(1.0, fader->args.duration));
    CLAMP(
        fader->current.value, MIN(fader->args.initial, fader->args.target),
        MAX(fader->args.initial, fader->args.target));
    fader->current.frame++;
    fader->is_active =
        fader->current.frame <= fader->args.duration + fader->args.debuff;

    return Fader_IsActive(fader);
}
