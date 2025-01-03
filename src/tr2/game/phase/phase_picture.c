#include "game/phase/phase_picture.h"

#include "game/console/common.h"
#include "game/input.h"
#include "game/output.h"
#include "game/phase/priv.h"
#include "global/vars.h"

#include <libtrx/game/fader.h>
#include <libtrx/memory.h>

typedef enum {
    STATE_FADE_IN,
    STATE_WAIT,
    STATE_FADE_OUT,
} M_STATE;

typedef struct {
    M_STATE state;
    FADER fader;
    int32_t frames;
    PHASE_PICTURE_ARGS args;
} M_PRIV;

static void M_FadeOut(M_PRIV *p);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

static void M_FadeOut(M_PRIV *const p)
{
    p->state = STATE_FADE_OUT;
    Fader_InitAnyToBlack(&p->fader, p->args.fade_out_time);
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Output_LoadBackgroundFromFile(p->args.file_name);
    Fader_InitBlackToTransparent(&p->fader, p->args.fade_in_time);
    return (PHASE_CONTROL) {};
}

static void M_End(PHASE *const phase)
{
    Output_UnloadBackground();
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;

    p->frames += num_frames;

    switch (p->state) {
    case STATE_FADE_IN:
        Input_Update();
        if (g_InputDB.menu_confirm || g_InputDB.menu_back || g_IsGameToExit) {
            M_FadeOut(p);
        } else if (!Fader_Control(&p->fader)) {
            p->state = STATE_WAIT;
        }
        break;

    case STATE_WAIT:
        Input_Update();
        if (g_InputDB.menu_confirm || g_InputDB.menu_back || g_IsGameToExit
            || p->frames >= p->args.display_time - p->args.fade_out_time) {
            M_FadeOut(p);
        }
        break;

    case STATE_FADE_OUT:
        Input_Update();
        if (g_InputDB.menu_confirm || g_InputDB.menu_back
            || !Fader_Control(&p->fader)) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        }
    }

    return (PHASE_CONTROL) {};
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Output_DrawBackground();
    Output_DrawPolyList();
    Output_DrawBlackRectangle(Fader_GetCurrentValue(&p->fader));
    Console_Draw();
    Text_Draw();
    Output_DrawPolyList();
}

PHASE *Phase_Picture_Create(const PHASE_PICTURE_ARGS args)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->args = args;
    p->state = STATE_FADE_IN;
    phase->priv = p;
    phase->start = M_Start;
    phase->end = M_End;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Picture_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
