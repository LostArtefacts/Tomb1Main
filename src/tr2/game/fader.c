#include "game/fader.h"

#include "config.h"
#include "game/input.h"
#include "game/shell.h"
#include "global/const.h"

#include <libtrx/utils.h>

void Fader_Init(
    FADER *const fader, const int32_t initial, const int32_t target,
    const int32_t duration, const int32_t debuff)
{
    fader->initial = initial;
    fader->target = target;
    fader->duration = duration;
    // This value controls how much to keep the last frame after the animation
    // is done.
    fader->debuff = debuff;
    fader->current.value = initial;
    fader->current.frame = 0;

    if (g_Config.visuals.enable_fade_effects && duration > 0) {
        fader->is_active = true;
    } else {
        fader->is_active = false;
        fader->current.frame = fader->duration + fader->debuff;
        fader->current.value = target;
    }
}

void Fader_InitBlackToTransparent(FADER *const fader, const int32_t duration)
{
    Fader_Init(fader, 255, 0, duration, 0);
}

void Fader_InitTransparentToBlack(FADER *const fader, const int32_t duration)
{
    Fader_Init(fader, 0, 255, duration, FRAMES_PER_SECOND / 6);
}

void Fader_InitAnyToBlack(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader, fader->current.value, 255, duration, FRAMES_PER_SECOND / 6);
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

    fader->current.value = fader->initial
        + (fader->target - fader->initial)
            * (fader->current.frame / (float)MAX(1.0, fader->duration));
    CLAMP(
        fader->current.value, MIN(fader->initial, fader->target),
        MAX(fader->initial, fader->target));
    fader->current.frame++;
    fader->is_active = fader->current.frame <= fader->duration + fader->debuff;

    Input_Update();
    Shell_ProcessInput();
    Shell_ProcessEvents();
    if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
        // cancel the fade immediately
        fader->is_active = false;
        fader->current.frame = fader->duration + fader->debuff;
        fader->current.value = fader->target;
        return false;
    }

    return Fader_IsActive(fader);
}
