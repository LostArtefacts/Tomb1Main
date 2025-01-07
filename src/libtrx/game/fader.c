#include "game/fader.h"

#include "config.h"
#include "game/clock.h"
#include "utils.h"

void Fader_Init(FADER *const fader, FADER_ARGS args)
{
    if (args.initial == FADER_ANY) {
        args.initial = Fader_GetCurrentValue(fader);

        // Reduce duration proportionally to how close the initial value is to
        // the target.
        double ratio = ABS(args.target - args.initial) / 128.0;
        CLAMP(ratio, 0.0, 1.0);
        args.duration *= ratio;
        if (ratio < 1.0) {
            args.debuff = 0;
        }
    }

    fader->args = args;
    ClockTimer_Sync(&fader->timer);
}

void Fader_InitBlackToTransparent(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = FADER_BLACK,
            .target = FADER_TRANSPARENT,
            .duration = duration,
            .debuff = 0,
        });
}

void Fader_InitTransparentToBlack(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = FADER_TRANSPARENT,
            .target = FADER_BLACK,
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
            .initial = FADER_TRANSPARENT,
            .target = FADER_SEMI_BLACK,
            .duration = duration,
            .debuff = LOGIC_FPS / 6,
        });
}

void Fader_InitAnyToBlack(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = FADER_ANY,
            .target = FADER_BLACK,
            .duration = duration,
            .debuff = LOGIC_FPS / 6,
        });
}

void Fader_InitAnyToSemiBlack(FADER *const fader, const int32_t duration)
{
    Fader_Init(
        fader,
        (FADER_ARGS) {
            .initial = FADER_ANY,
            .target = FADER_SEMI_BLACK,
            .duration = duration,
            .debuff = LOGIC_FPS / 6,
        });
}

int32_t Fader_GetCurrentValue(const FADER *const fader)
{
    if (!g_Config.visuals.enable_fade_effects || fader->args.duration == 0) {
        return FADER_TRANSPARENT;
    }
    const double elapsed_time = ClockTimer_PeekElapsed(&fader->timer);
    const double target_time = fader->args.duration / (double)LOGIC_FPS;
    double ratio = elapsed_time / target_time;
    CLAMP(ratio, 0.0, 1.0);
    return fader->args.initial
        + (fader->args.target - fader->args.initial) * ratio;
}

bool Fader_IsActive(const FADER *const fader)
{
    if (!g_Config.visuals.enable_fade_effects) {
        return false;
    }
    const int32_t elapsed_frames =
        ClockTimer_PeekElapsed(&fader->timer) * LOGIC_FPS;
    const int32_t target_frames = fader->args.duration + fader->args.debuff;
    return elapsed_frames < target_frames;
}

bool Fader_Control(FADER *const fader)
{
    return Fader_IsActive(fader);
}
