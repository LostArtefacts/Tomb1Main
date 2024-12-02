#include "game/game.h"

#include "config.h"
#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/camera.h"
#include "game/console/common.h"
#include "game/demo.h"
#include "game/gameflow/gameflow_new.h"
#include "game/input.h"
#include "game/inventory/backpack.h"
#include "game/inventory/common.h"
#include "game/lara/control.h"
#include "game/music.h"
#include "game/overlay.h"
#include "game/room_draw.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "game/text.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

static int32_t m_FrameCount = 0;

int32_t __cdecl Game_Control(int32_t nframes, const bool demo_mode)
{
    if (g_GF_OverrideDir != (GAME_FLOW_DIR)-1) {
        return GFD_OVERRIDE;
    }

    CLAMPG(nframes, MAX_FRAMES);
    m_FrameCount += nframes;

    if (m_FrameCount <= 0) {
        return 0;
    }

    while (m_FrameCount > 0) {
        if (!g_GameFlow.cheat_mode_check_disabled) {
            CheckCheatMode();
        }

        if (g_LevelComplete) {
            return GFD_START_GAME | LV_FIRST;
        }

        Shell_ProcessEvents();
        Input_Update();
        Shell_ProcessInput();
        Game_ProcessInput();

        if (demo_mode) {
            if (g_InputDB.any) {
                return g_GameFlow.on_demo_interrupt;
            }
            if (!Demo_GetInput()) {
                g_Input = (INPUT_STATE) { 0 };
                return g_GameFlow.on_demo_end;
            }
        } else if (g_GameFlow.no_input_timeout) {
            if (g_InputDB.any) {
                g_NoInputCounter = 0;
            } else {
                g_NoInputCounter++;
                if (g_NoInputCounter > g_GameFlow.no_input_time) {
                    return GFD_START_DEMO;
                }
            }
        }

        if (g_Lara.death_timer > DEATH_WAIT
            || (g_Lara.death_timer > DEATH_WAIT_INPUT && g_Input.any)
            || g_OverlayStatus == 2) {
            if (demo_mode) {
                return g_GameFlow.on_death_demo_mode;
            }
            if (g_CurrentLevel == LV_GYM) {
                return GFD_EXIT_TO_TITLE;
            }
            if (g_GameFlow.on_death_in_game) {
                return g_GameFlow.on_death_in_game;
            }
            if (g_OverlayStatus == 2) {
                g_OverlayStatus = 1;
                const GAME_FLOW_DIR dir = Inv_Display(INV_DEATH_MODE);
                if (dir != 0) {
                    return dir;
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
                GAME_FLOW_DIR dir;
                if (g_OverlayStatus == -1) {
                    dir = Inv_Display(INV_LOAD_MODE);
                } else if (g_OverlayStatus == -2) {
                    dir = Inv_Display(INV_SAVE_MODE);
                } else {
                    dir = Inv_Display(INV_GAME_MODE);
                }
                if (g_GF_OverrideDir != (GAME_FLOW_DIR)-1) {
                    return GFD_OVERRIDE;
                }
                g_OverlayStatus = 1;

                if (dir != 0) {
                    if (g_Inv_ExtraData[0] == 1) {
                        if (g_CurrentLevel == LV_GYM) {
                            return GFD_START_GAME | LV_FIRST;
                        }
                        CreateSaveGameInfo();
                        const int16_t slot_num = g_Inv_ExtraData[1];
                        S_SaveGame(
                            &g_SaveGame, sizeof(SAVEGAME_INFO), slot_num);
                        S_SaveSettings();
                    } else {
                        return dir;
                    }
                }
            }
        }

        g_DynamicLightCount = 0;

        {
            int16_t item_num = g_NextItemActive;
            while (item_num != NO_ITEM) {
                ITEM *const item = &g_Items[item_num];
                const int16_t next = item->next_active;
                const OBJECT *object = &g_Objects[item->object_id];
                if (!(item->flags & IF_KILLED) && object->control != NULL) {
                    object->control(item_num);
                }
                item_num = next;
            }
        }

        {
            int16_t fx_num = g_NextEffectActive;
            while (fx_num != NO_ITEM) {
                FX *const fx = &g_Effects[fx_num];
                const OBJECT *const object = &g_Objects[fx->object_id];
                const int32_t next = fx->next_active;
                if (object->control != NULL) {
                    object->control(fx_num);
                }
                fx_num = next;
            }
        }

        Lara_Control(0);
        HairControl(0);
        Camera_Update();
        Sound_UpdateEffects();

        g_HealthBarTimer--;
        if (g_CurrentLevel || g_IsAssaultTimerActive) {
            Stats_UpdateTimer();
        }

        m_FrameCount -= 2;
    }

    return 0;
}

int32_t __cdecl Game_Draw(void)
{
    Room_DrawAllRooms(g_Camera.pos.room_num);
    Overlay_DrawGameInfo(true);
    S_OutputPolyList();
    const int32_t frames = S_DumpScreen();
    g_Camera.num_frames = frames * TICKS_PER_FRAME;
    Shell_ProcessEvents();
    Overlay_Animate(frames);
    S_AnimateTextures(g_Camera.num_frames);
    return g_Camera.num_frames;
}

int32_t __cdecl Game_DrawCinematic(void)
{
    g_CameraUnderwater = false;
    Room_DrawAllRooms(g_Camera.pos.room_num);
    Console_Draw();
    Text_Draw();
    S_OutputPolyList();
    g_Camera.num_frames = S_DumpScreen();
    Shell_ProcessEvents();
    S_AnimateTextures(g_Camera.num_frames);
    return g_Camera.num_frames;
}

int16_t __cdecl Game_Start(
    const int32_t level_num, const GAMEFLOW_LEVEL_TYPE level_type)
{
    if (level_type == GFL_NORMAL || level_type == GFL_SAVED
        || level_type == GFL_DEMO) {
        g_CurrentLevel = level_num;
    }
    if (level_type != GFL_SAVED) {
        ModifyStartInfo(level_num);
    }
    g_IsTitleLoaded = false;
    if (level_type != GFL_SAVED) {
        InitialiseLevelFlags();
    }
    if (!Level_Initialise(level_num, level_type)) {
        g_CurrentLevel = 0;
        return GFD_EXIT_GAME;
    }

    GAME_FLOW_DIR dir = Game_Loop(false);
    if (dir == GFD_OVERRIDE) {
        dir = g_GF_OverrideDir;
        g_GF_OverrideDir = (GAME_FLOW_DIR)-1;
        return dir;
    }
    if (dir == GFD_EXIT_TO_TITLE || dir == GFD_START_DEMO) {
        return dir;
    }

    if (dir == GFD_EXIT_GAME) {
        g_CurrentLevel = 0;
        return dir;
    }

    if (g_LevelComplete) {
        if (g_GameFlow.demo_version && g_GameFlow.single_level) {
            return GFD_EXIT_TO_TITLE;
        }

        if (g_CurrentLevel == LV_GYM) {
            S_FadeToBlack();
            return GFD_EXIT_TO_TITLE;
        }

        S_FadeInInventory(1);
        return GFD_LEVEL_COMPLETE | g_CurrentLevel;
    }

    S_FadeToBlack();
    if (!g_Inv_Chosen) {
        return GFD_EXIT_TO_TITLE;
    }

    if (g_Inv_ExtraData[0] == 0) {
        S_LoadGame(&g_SaveGame, sizeof(SAVEGAME_INFO), g_Inv_ExtraData[1]);
        return GFD_START_SAVED_GAME | g_Inv_ExtraData[1];
    }

    if (g_Inv_ExtraData[0] != 1) {
        return GFD_EXIT_TO_TITLE;
    }

    if (g_GameFlow.play_any_level) {
        return g_Inv_ExtraData[1] + 1;
    }

    return GFD_START_GAME | LV_FIRST;
}

int32_t __cdecl Game_Loop(const bool demo_mode)
{
    g_OverlayStatus = 1;
    Camera_Initialise();
    g_NoInputCounter = 0;
    g_GameMode = demo_mode ? GAMEMODE_IN_DEMO : GAMEMODE_IN_GAME;

    Stats_StartTimer();
    GAME_FLOW_DIR dir = Game_Control(1, demo_mode);
    while (dir == 0) {
        const int32_t nframes = Game_Draw();
        if (g_IsGameToExit) {
            dir = GFD_EXIT_GAME;
        } else {
            dir = Game_Control(nframes, demo_mode);
        }
    }

    g_GameMode = GAMEMODE_NOT_IN_GAME;

    Overlay_HideGameInfo();
    Sound_StopAllSamples();
    Music_Stop();

    Music_SetVolume(g_Config.audio.music_volume);

    return dir;
}

GAMEFLOW_LEVEL_TYPE Game_GetCurrentLevelType(void)
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
