#include "game/game.h"

#include "game/camera.h"
#include "game/clock.h"
#include "game/effects.h"
#include "game/game_flow.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/inventory.h"
#include "game/item_actions.h"
#include "game/lara/cheat.h"
#include "game/lara/common.h"
#include "game/lara/hair.h"
#include "game/level.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/savegame.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/debug.h>
#include <libtrx/game/ui/common.h>

#define FRAME_BUFFER(key)                                                      \
    do {                                                                       \
        Output_BeginScene();                                                   \
        Game_Draw(true);                                                       \
        Input_Update();                                                        \
        Output_EndScene();                                                     \
        Clock_WaitTick();                                                      \
    } while (g_Input.key);

void Game_ProcessInput(void)
{
    if (g_Config.input.enable_numeric_keys) {
        if (g_InputDB.equip_pistols && Inv_RequestItem(O_PISTOL_ITEM)) {
            g_Lara.request_gun_type = LGT_PISTOLS;
        } else if (g_InputDB.equip_shotgun && Inv_RequestItem(O_SHOTGUN_ITEM)) {
            g_Lara.request_gun_type = LGT_SHOTGUN;
        } else if (g_InputDB.equip_magnums && Inv_RequestItem(O_MAGNUM_ITEM)) {
            g_Lara.request_gun_type = LGT_MAGNUMS;
        } else if (g_InputDB.equip_uzis && Inv_RequestItem(O_UZI_ITEM)) {
            g_Lara.request_gun_type = LGT_UZIS;
        }
    }

    if (g_InputDB.use_small_medi && Inv_RequestItem(O_MEDI_OPTION)) {
        Lara_UseItem(O_MEDI_OPTION);
    } else if (g_InputDB.use_big_medi && Inv_RequestItem(O_BIGMEDI_OPTION)) {
        Lara_UseItem(O_BIGMEDI_OPTION);
    }

    if (g_Config.input.enable_buffering && Game_IsPlaying()) {
        if (g_Input.toggle_bilinear_filter) {
            FRAME_BUFFER(toggle_bilinear_filter);
        } else if (g_Input.toggle_perspective_filter) {
            FRAME_BUFFER(toggle_perspective_filter);
        } else if (g_Input.toggle_fps_counter) {
            FRAME_BUFFER(toggle_fps_counter);
        }
    }

    if (g_InputDB.toggle_ui) {
        UI_ToggleState(&g_Config.ui.enable_game_ui);
    }
}

bool Game_Start_Legacy(
    const GAME_FLOW_LEVEL *const level,
    const GAME_FLOW_SEQUENCE_CONTEXT seq_ctx)
{
    g_GameInfo.current_level_type = level->type;

    switch (seq_ctx) {
    case GFSC_SAVED:
        // reset current info to the defaults so that we do not do
        // Item_GlobalReplace in the inventory initialization routines too early
        Savegame_InitCurrentInfo();

        if (!Level_Initialise(level)) {
            Game_SetCurrentLevel(NULL);
            GF_SetCurrentLevel(NULL);
            return false;
        }
        if (!Savegame_Load(g_GameInfo.current_save_slot)) {
            LOG_ERROR("Failed to load save file!");
            return false;
        }
        break;

    case GFSC_RESTART:
        if (level->num <= g_GameFlow.first_level_num) {
            Savegame_InitCurrentInfo();
        } else {
            Savegame_ResetCurrentInfo(level->num);
            // Use previous level's ending info to start current level.
            Savegame_CarryCurrentInfoToNextLevel(level->num - 1, level->num);
            Savegame_ApplyLogicToCurrentInfo(level->num);
        }
        if (!Level_Initialise(level)) {
            Game_SetCurrentLevel(NULL);
            GF_SetCurrentLevel(NULL);
            return false;
        }
        break;

    case GFSC_SELECT:
        if (g_GameInfo.current_save_slot != -1) {
            // select level feature
            Savegame_InitCurrentInfo();
            if (level->num > g_GameFlow.first_level_num) {
                Savegame_LoadOnlyResumeInfo(
                    g_GameInfo.current_save_slot, &g_GameInfo);
                for (int i = level->num; i < g_GameFlow.level_count; i++) {
                    Savegame_ResetCurrentInfo(i);
                }
                // Use previous level's ending info to start current level.
                Savegame_CarryCurrentInfoToNextLevel(
                    level->num - 1, level->num);
                Savegame_ApplyLogicToCurrentInfo(level->num);
            }
        } else {
            // console /play level feature
            Savegame_InitCurrentInfo();
            for (int i = g_GameFlow.first_level_num + 1; i <= level->num; i++) {
                Savegame_CarryCurrentInfoToNextLevel(i - 1, i);
                Savegame_ApplyLogicToCurrentInfo(i);
            }
        }
        if (!Level_Initialise(level)) {
            return false;
        }
        break;

    default:
        if (level->type == GFL_GYM) {
            Savegame_ResetCurrentInfo(level->num);
            Savegame_ApplyLogicToCurrentInfo(level->num);
        } else if (level->type == GFL_BONUS) {
            Savegame_CarryCurrentInfoToNextLevel(level->num - 1, level->num);
            Savegame_ApplyLogicToCurrentInfo(level->num);
        }
        if (!Level_Initialise(level)) {
            return false;
        }
        break;
    }

    // LaraGun() expects request_gun_type to be set only when it
    // really is needed, not at all times.
    // https://github.com/LostArtefacts/TRX/issues/36
    g_Lara.request_gun_type = LGT_UNARMED;

    g_OverlayFlag = 1;
    Camera_Initialise();

    Stats_CalculateStats();
    g_GameInfo.current[g_CurrentLevel].stats.max_pickup_count =
        Stats_GetPickups();
    g_GameInfo.current[g_CurrentLevel].stats.max_kill_count =
        Stats_GetKillables();
    g_GameInfo.current[g_CurrentLevel].stats.max_secret_count =
        Stats_GetSecrets();

    g_GameInfo.ask_for_save = g_Config.gameplay.enable_save_crystals
        && (level->type == GFL_NORMAL || level->type == GFL_BONUS)
        && g_CurrentLevel != g_GameFlow.first_level_num
        && g_CurrentLevel != g_GameFlow.gym_level_num;

    Interpolation_Remember();
    return true;
}

GAME_FLOW_COMMAND Game_Stop_Legacy(void)
{
    Sound_StopAll();
    Music_Stop();
    Savegame_PersistGameToCurrentInfo(g_CurrentLevel);

    if (g_CurrentLevel == g_GameFlow.last_level_num) {
        g_Config.profile.new_game_plus_unlock = true;
        Config_Write();
        g_GameInfo.bonus_level_unlock =
            Stats_CheckAllSecretsCollected(GFL_NORMAL);
    }

    // play specific level
    if (g_LevelComplete && g_GameInfo.select_level_num != -1) {
        if (g_CurrentLevel != -1) {
            Savegame_CarryCurrentInfoToNextLevel(
                g_CurrentLevel, g_GameInfo.select_level_num);
        }
        return (GAME_FLOW_COMMAND) {
            .action = GF_SELECT_GAME,
            .param = g_GameInfo.select_level_num,
        };
    }

    // carry info to the next level
    if (g_CurrentLevel + 1 < g_GameFlow.level_count) {
        // TODO: this should be moved to GFS_LEVEL_COMPLETE handler, probably
        Savegame_CarryCurrentInfoToNextLevel(
            g_CurrentLevel, g_CurrentLevel + 1);
        Savegame_ApplyLogicToCurrentInfo(g_CurrentLevel + 1);
    }

    // normal level completion
    if (g_LevelComplete) {
        // TODO: why is this made unavailable?
        g_GameInfo.current[g_CurrentLevel].flags.available = 0;
        return (GAME_FLOW_COMMAND) {
            .action = GF_LEVEL_COMPLETE,
            .param = g_GameInfo.select_level_num,
        };
    }

    if (g_GameInfo.passport_selection == PASSPORT_MODE_LOAD_GAME) {
        return (GAME_FLOW_COMMAND) {
            .action = GF_START_SAVED_GAME,
            .param = g_GameInfo.current_save_slot,
        };
    } else if (g_GameInfo.passport_selection == PASSPORT_MODE_SELECT_LEVEL) {
        return (GAME_FLOW_COMMAND) {
            .action = GF_SELECT_GAME,
            .param = g_GameInfo.select_level_num,
        };
    } else if (g_GameInfo.passport_selection == PASSPORT_MODE_STORY_SO_FAR) {
        return (GAME_FLOW_COMMAND) {
            .action = GF_STORY_SO_FAR,
            .param = g_GameInfo.current_save_slot,
        };
    } else if (g_GameInfo.passport_selection == PASSPORT_MODE_RESTART) {
        return (GAME_FLOW_COMMAND) {
            .action = GF_RESTART_GAME,
            .param = g_CurrentLevel,
        };
    } else if (g_GameInfo.passport_selection == PASSPORT_MODE_NEW_GAME) {
        Savegame_InitCurrentInfo();
        return (GAME_FLOW_COMMAND) {
            .action = GF_START_GAME,
            .param = g_GameFlow.first_level_num,
        };
    } else {
        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
}

bool Game_Start(
    const GAME_FLOW_LEVEL *const level,
    const GAME_FLOW_SEQUENCE_CONTEXT seq_ctx)
{
    Game_SetCurrentLevel(level);
    Interpolation_Remember();
    Stats_StartTimer();
    return true;
}

void Game_End(void)
{
}

void Game_Suspend(void)
{
}

void Game_Resume(void)
{
}

GAME_FLOW_COMMAND Game_Control(const bool demo_mode)
{
    ASSERT(!demo_mode);

    if (g_GameInfo.ask_for_save) {
        // ask for a save at the start of a level for the save crystals mode
        const GAME_FLOW_COMMAND gf_cmd =
            GF_ShowInventory(INV_SAVE_CRYSTAL_MODE);
        g_GameInfo.ask_for_save = false;
        if (gf_cmd.action != GF_NOOP) {
            return gf_cmd;
        }
    }

    Interpolation_Remember();
    Stats_UpdateTimer();

    Lara_Cheat_Control();
    if (g_LevelComplete) {
        return Game_Stop_Legacy();
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
            return GF_ShowInventory(INV_DEATH_MODE);
        } else {
            g_OverlayFlag = 2;
        }
    }

    if ((g_InputDB.option || g_Input.save || g_Input.load || g_OverlayFlag <= 0)
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
            return gf_cmd;
        }
    }

    if (!g_Lara.death_timer && g_InputDB.pause) {
        return GF_PauseGame();
    } else if (g_InputDB.toggle_photo_mode) {
        return GF_EnterPhotoMode();
    } else {
        Output_ResetDynamicLights();

        Item_Control();
        Effect_Control();

        Lara_Control();
        Lara_Hair_Control();

        Camera_Update();
        Sound_ResetAmbient();
        ItemAction_RunActive();
        Sound_UpdateEffects();
        Overlay_BarHealthTimerTick();
        Output_AnimateTextures(1);
    }
    return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
}

GAME_FLOW_LEVEL_TYPE Game_GetCurrentLevelType(void)
{
    return g_GameInfo.current_level_type;
}

extern int32_t Game_GetCurrentLevelNum(void)
{
    return g_CurrentLevel;
}

bool Game_IsInGym(void)
{
    const GAME_FLOW_LEVEL *const current_level = GF_GetCurrentLevel();
    return current_level != NULL && current_level->type == GFL_GYM;
}

bool Game_IsPlayable(void)
{
    if (g_GameInfo.current_level_type == GFL_TITLE
        || g_GameInfo.current_level_type == GFL_DEMO
        || g_GameInfo.current_level_type == GFL_CUTSCENE) {
        return false;
    }

    if (!g_Objects[O_LARA].loaded || g_LaraItem == NULL) {
        return false;
    }

    return true;
}
