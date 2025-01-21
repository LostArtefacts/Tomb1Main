#include "game/game.h"

#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/camera.h"
#include "game/demo.h"
#include "game/effects.h"
#include "game/game_flow.h"
#include "game/inventory.h"
#include "game/item_actions.h"
#include "game/lara/cheat_keys.h"
#include "game/lara/control.h"
#include "game/lara/hair.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/room_draw.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>

bool Game_Start(const int32_t level_num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    if (level_type == GFL_NORMAL || level_type == GFL_SAVED
        || level_type == GFL_DEMO) {
        g_CurrentLevel = level_num;
    }
    if (level_type != GFL_SAVED) {
        ModifyStartInfo(level_num);
        InitialiseLevelFlags();
    }
    if (!Level_Initialise(level_num, level_type)) {
        g_CurrentLevel = 0;
        return false;
    }

    g_OverlayStatus = 1;
    Camera_Initialise();
    Stats_StartTimer();
    return true;
}

void Game_End(void)
{
    Overlay_HideGameInfo();
    Sound_StopAll();
    Music_Stop();
    Music_SetVolume(g_Config.audio.music_volume);
}

void Game_Suspend(void)
{
}

void Game_Resume(void)
{
    Stats_StartTimer();
}

GAME_FLOW_COMMAND Game_Control(const bool demo_mode)
{
    if (!g_GameFlow.cheat_keys) {
        Lara_Cheat_CheckKeys();
    }

    if (g_LevelComplete) {
        if (g_GameFlow.is_demo_version && g_GameFlow.single_level) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        if (g_CurrentLevel == LV_GYM) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        return (GAME_FLOW_COMMAND) {
            .action = GF_LEVEL_COMPLETE,
            .param = g_CurrentLevel,
        };
    }

    Input_Update();
    Shell_ProcessInput();
    Game_ProcessInput();

    if (g_InputDB.toggle_photo_mode) {
        return GF_EnterPhotoMode();
    }
    if (g_InputDB.pause) {
        return GF_PauseGame();
    }

    if (demo_mode) {
        if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
            return g_GameFlow.on_demo_interrupt;
        }
        if (!Demo_GetInput()) {
            g_Input = (INPUT_STATE) {};
            return g_GameFlow.on_demo_end;
        }
    }

    if (g_Lara.death_timer > DEATH_WAIT
        || (g_Lara.death_timer > DEATH_WAIT_INPUT && g_Input.any)
        || g_OverlayStatus == 2) {
        if (demo_mode) {
            return g_GameFlow.on_death_demo_mode;
        }
        if (g_CurrentLevel == LV_GYM) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        if (g_GameFlow.on_death_in_game.action != GF_NOOP) {
            return g_GameFlow.on_death_in_game;
        }
        if (g_OverlayStatus == 2) {
            g_OverlayStatus = 1;
            const GAME_FLOW_COMMAND gf_cmd = GF_ShowInventory(INV_DEATH_MODE);
            if (gf_cmd.action != GF_NOOP) {
                return gf_cmd;
            }
        } else {
            g_OverlayStatus = 2;
        }
    }

    if (((g_InputDB.load || g_InputDB.save || g_InputDB.option)
         || g_OverlayStatus <= 0)
        && g_Lara.death_timer == 0 && !g_Lara.extra_anim) {
        if (g_OverlayStatus > 0) {
            if (g_GameFlow.load_save_disabled) {
                g_OverlayStatus = 0;
            } else if (g_Input.load) {
                g_OverlayStatus = -1;
            } else {
                g_OverlayStatus = g_Input.save ? -2 : 0;
            }
        } else {
            GAME_FLOW_COMMAND gf_cmd;
            if (g_OverlayStatus == -1) {
                gf_cmd = GF_ShowInventory(INV_LOAD_MODE);
            } else if (g_OverlayStatus == -2) {
                gf_cmd = GF_ShowInventory(INV_SAVE_MODE);
            } else {
                gf_cmd = GF_ShowInventory(INV_GAME_MODE);
            }
            g_OverlayStatus = 1;
            if (gf_cmd.action != GF_NOOP) {
                return gf_cmd;
            }
        }
    }

    g_DynamicLightCount = 0;

    Item_Control();
    Effect_Control();
    Lara_Control(false);
    Lara_Hair_Control(false);
    Camera_Update();
    Sound_UpdateEffects();
    Sound_EndScene();
    ItemAction_RunActive();
    Overlay_Animate(1);
    Output_AnimateTextures(1 * TICKS_PER_FRAME);

    g_HealthBarTimer--;
    if (g_CurrentLevel != LV_GYM || g_IsAssaultTimerActive) {
        Stats_UpdateTimer();
    }

    return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
}

void Game_Draw(bool draw_overlay)
{
    Camera_Apply();
    Room_DrawAllRooms(g_Camera.pos.room_num);
    Output_DrawPolyList();
    if (draw_overlay) {
        Overlay_DrawGameInfo();
        Output_DrawPolyList();
    } else {
        Overlay_HideGameInfo();
    }
}

GAME_FLOW_LEVEL_TYPE Game_GetCurrentLevelType(void)
{
    return g_GameInfo.current_level.type;
}

extern int32_t Game_GetCurrentLevelNum(void)
{
    return g_CurrentLevel;
}

bool Game_IsPlayable(void)
{
    if (g_GameInfo.current_level.type == GFL_TITLE
        || g_GameInfo.current_level.type == GFL_DEMO
        || g_GameInfo.current_level.type == GFL_CUTSCENE) {
        return false;
    }

    if (!g_Objects[O_LARA].loaded || g_LaraItem == NULL) {
        return false;
    }

    return true;
}

void Game_ProcessInput(void)
{
    if (g_GameInfo.current_level.type == GFL_DEMO) {
        return;
    }

    if (g_InputDB.equip_pistols && Inv_RequestItem(O_PISTOL_OPTION)) {
        g_Lara.request_gun_type = LGT_PISTOLS;
    } else if (g_InputDB.equip_shotgun && Inv_RequestItem(O_SHOTGUN_OPTION)) {
        g_Lara.request_gun_type = LGT_SHOTGUN;
    } else if (g_InputDB.equip_magnums && Inv_RequestItem(O_MAGNUM_OPTION)) {
        g_Lara.request_gun_type = LGT_MAGNUMS;
    } else if (g_InputDB.equip_uzis && Inv_RequestItem(O_UZI_OPTION)) {
        g_Lara.request_gun_type = LGT_UZIS;
    } else if (g_InputDB.equip_harpoon && Inv_RequestItem(O_HARPOON_OPTION)) {
        g_Lara.request_gun_type = LGT_HARPOON;
    } else if (g_InputDB.equip_m16 && Inv_RequestItem(O_M16_OPTION)) {
        g_Lara.request_gun_type = LGT_M16;
    } else if (
        g_InputDB.equip_grenade_launcher && Inv_RequestItem(O_GRENADE_OPTION)) {
        g_Lara.request_gun_type = LGT_GRENADE;
    }

    if (g_InputDB.use_small_medi && Inv_RequestItem(O_SMALL_MEDIPACK_OPTION)) {
        Lara_UseItem(O_SMALL_MEDIPACK_OPTION);
    }
    if (g_InputDB.use_big_medi && Inv_RequestItem(O_LARGE_MEDIPACK_OPTION)) {
        Lara_UseItem(O_LARGE_MEDIPACK_OPTION);
    }

    if (g_GameFlow.load_save_disabled) {
        g_Input.save = 0;
        g_Input.load = 0;
    }
}
