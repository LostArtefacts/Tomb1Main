#include "game/phase/phase_game.h"

#include "game/camera.h"
#include "game/effects.h"
#include "game/game.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/item_actions.h"
#include "game/items.h"
#include "game/lara/cheat.h"
#include "game/lara/common.h"
#include "game/lara/hair.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/phase.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "game/text.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/memory.h>
#include <libtrx/utils.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static void M_Start(const void *args);
static void M_End(void);
static PHASE_CONTROL M_Control(int32_t nframes);
static void M_Draw(void);

static void M_Start(const void *const args)
{
    Interpolation_Remember();
    Stats_StartTimer();
    Game_SetIsPlaying(true);
}

static void M_End(void)
{
    Game_SetIsPlaying(false);
}

static PHASE_CONTROL M_Control(int32_t nframes)
{
    Interpolation_Remember();
    Stats_UpdateTimer();
    CLAMPG(nframes, MAX_FRAMES);

    for (int32_t i = 0; i < nframes; i++) {
        Lara_Cheat_Control();
        if (g_LevelComplete) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        }

        Input_Update();
        Shell_ProcessInput();
        Game_ProcessInput();

        if (g_Lara.death_timer > DEATH_WAIT
            || (g_Lara.death_timer > DEATH_WAIT_MIN && g_Input.any
                && !g_Input.fly_cheat)
            || g_OverlayFlag == 2) {
            if (g_OverlayFlag == 2) {
                g_OverlayFlag = 1;
                const GAME_FLOW_COMMAND gf_cmd =
                    GF_ShowInventory(INV_DEATH_MODE);
                if (gf_cmd.action != GF_NOOP) {
                    return (PHASE_CONTROL) {
                        .action = PHASE_ACTION_END,
                        .gf_cmd = gf_cmd,
                    };
                }
                return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
            } else {
                g_OverlayFlag = 2;
            }
        }

        if ((g_InputDB.option || g_Input.save || g_Input.load
             || g_OverlayFlag <= 0)
            && !g_Lara.death_timer) {
            if (g_Camera.type == CAM_CINEMATIC) {
                g_OverlayFlag = 0;
            } else if (g_OverlayFlag > 0) {
                if (g_Input.load) {
                    g_OverlayFlag = -1;
                } else if (g_Input.save) {
                    g_OverlayFlag = -2;
                } else {
                    g_OverlayFlag = 0;
                }
            } else {
                GAME_FLOW_COMMAND gf_cmd;
                if (g_OverlayFlag == -1) {
                    gf_cmd = GF_ShowInventory(INV_LOAD_MODE);
                } else if (g_OverlayFlag == -2) {
                    gf_cmd = GF_ShowInventory(INV_SAVE_MODE);
                } else {
                    gf_cmd = GF_ShowInventory(INV_GAME_MODE);
                }
                g_OverlayFlag = 1;
                if (gf_cmd.action != GF_NOOP) {
                    return (PHASE_CONTROL) {
                        .action = PHASE_ACTION_END,
                        .gf_cmd = gf_cmd,
                    };
                }
                return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
            }
        }

        if (!g_Lara.death_timer && g_InputDB.pause) {
            Game_SetIsPlaying(false);
            PHASE *const phase_pause = Phase_Pause_Create();
            const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase_pause);
            Phase_Pause_Destroy(phase_pause);
            Game_SetIsPlaying(true);
            if (gf_cmd.action != GF_NOOP) {
                return (PHASE_CONTROL) {
                    .action = PHASE_ACTION_END,
                    .gf_cmd = gf_cmd,
                };
            }
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        } else if (g_InputDB.toggle_photo_mode) {
            Game_SetIsPlaying(false);
            PHASE *const subphase = Phase_PhotoMode_Create();
            const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
            Phase_PhotoMode_Destroy(subphase);
            Game_SetIsPlaying(true);
            if (gf_cmd.action != GF_NOOP) {
                return (PHASE_CONTROL) {
                    .action = PHASE_ACTION_END,
                    .gf_cmd = gf_cmd,
                };
            }
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        } else {
            Item_Control();
            Effect_Control();

            Lara_Control();
            Lara_Hair_Control();

            Camera_Update();
            Sound_ResetAmbient();
            ItemAction_RunActive();
            Sound_UpdateEffects();
            Overlay_BarHealthTimerTick();
        }
    }

    if (g_GameInfo.ask_for_save) {
        const GAME_FLOW_COMMAND gf_cmd =
            GF_ShowInventory(INV_SAVE_CRYSTAL_MODE);
        g_GameInfo.ask_for_save = false;
        if (gf_cmd.action != GF_NOOP) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = gf_cmd,
            };
        }
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(void)
{
    Game_DrawScene(true);
    Output_AnimateTextures();
    Text_Draw();
}

PHASER g_GamePhaser = {
    .start = M_Start,
    .end = M_End,
    .control = M_Control,
    .draw = M_Draw,
    .wait = NULL,
};
