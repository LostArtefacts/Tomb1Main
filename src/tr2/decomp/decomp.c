#include "decomp/decomp.h"

#include "config.h"
#include "decomp/savegame.h"
#include "game/clock.h"
#include "game/console/common.h"
#include "game/effects.h"
#include "game/fader.h"
#include "game/game.h"
#include "game/gamebuf.h"
#include "game/gameflow.h"
#include "game/gameflow/gameflow_new.h"
#include "game/input.h"
#include "game/inventory/backpack.h"
#include "game/inventory/common.h"
#include "game/items.h"
#include "game/lara/control.h"
#include "game/lara/draw.h"
#include "game/level.h"
#include "game/lot.h"
#include "game/math.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/random.h"
#include "game/requester.h"
#include "game/room.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/text.h"
#include "game/viewport.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/engine/image.h>
#include <libtrx/filesystem.h>
#include <libtrx/game/ui/common.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>
#include <libtrx/virtual_file.h>

#include <SDL2/SDL.h>
#include <stdio.h>

#define IDI_MAINICON 100

int32_t __stdcall WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,
    int32_t nShowCmd)
{
    g_GameModule = hInstance;
    g_CmdLine = lpCmdLine;
    HWND game_window = WinVidFindGameWindow();
    if (game_window) {
        SetForegroundWindow(game_window);
        return 0;
    }

    g_AppResultCode = 0;
    g_IsGameToExit = false;
    Shell_Setup();
    Shell_Main();
    Shell_Shutdown();

cleanup:
    return g_AppResultCode;
}

int16_t __cdecl TitleSequence(void)
{
    GF_N_LoadStrings(-1);

    g_NoInputCounter = 0;

    if (!Level_Initialise(0, GFL_TITLE)) {
        return GFD_EXIT_GAME;
    }

    if (g_GameFlow.title_track) {
        Music_Play(g_GameFlow.title_track, MPM_LOOPED);
    }

    GAME_FLOW_DIR dir = Inv_Display(INV_TITLE_MODE);
    Output_UnloadBackground();
    Music_Stop();

    if (dir == GFD_OVERRIDE) {
        dir = g_GF_OverrideDir;
        g_GF_OverrideDir = (GAME_FLOW_DIR)-1;
        return dir;
    }

    if (dir == GFD_START_DEMO) {
        return GFD_START_DEMO;
    }

    if (g_Inv_Chosen == O_PHOTO_OPTION) {
        return GFD_START_GAME | LV_GYM;
    }

    if (g_Inv_Chosen == O_PASSPORT_OPTION) {
        const int32_t slot_num = g_Inv_ExtraData[1];

        if (g_Inv_ExtraData[0] == 0) {
            Inv_RemoveAllItems();
            S_LoadGame(&g_SaveGame, sizeof(SAVEGAME_INFO), slot_num);
            return GFD_START_SAVED_GAME | slot_num;
        }

        if (g_Inv_ExtraData[0] == 1) {
            InitialiseStartInfo();
            int32_t level_id = LV_FIRST;
            if (g_GameFlow.play_any_level) {
                level_id = LV_FIRST + slot_num;
            }
            return GFD_START_GAME | level_id;
        }
        return GFD_EXIT_GAME;
    }

    return GFD_EXIT_GAME;
}

HWND __cdecl WinVidFindGameWindow(void)
{
    return FindWindowA(CLASS_NAME, WINDOW_NAME);
}

void __cdecl Game_SetCutsceneTrack(const int32_t track)
{
    g_CineTrackID = track;
}

void __cdecl CutscenePlayer_Control(const int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    item->rot.y = g_Camera.target_angle;
    item->pos.x = g_Camera.pos.pos.x;
    item->pos.y = g_Camera.pos.pos.y;
    item->pos.z = g_Camera.pos.pos.z;

    XYZ_32 pos = { 0 };
    Collide_GetJointAbsPosition(item, &pos, 0);

    const int16_t room_num = Room_FindByPos(pos.x, pos.y, pos.z);
    if (room_num != NO_ROOM_NEG && item->room_num != room_num) {
        Item_NewRoom(item_num, room_num);
    }

    if (item->dynamic_light && item->status != IS_INVISIBLE) {
        pos.x = 0;
        pos.y = 0;
        pos.z = 0;
        Collide_GetJointAbsPosition(item, &pos, 0);
        AddDynamicLight(pos.x, pos.y, pos.z, 12, 11);
    }

    Item_Animate(item);
}

void __cdecl Lara_Control_Cutscene(const int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    item->rot.y = g_Camera.target_angle;
    item->pos.x = g_Camera.pos.pos.x;
    item->pos.y = g_Camera.pos.pos.y;
    item->pos.z = g_Camera.pos.pos.z;

    XYZ_32 pos = { 0 };
    Collide_GetJointAbsPosition(item, &pos, 0);

    const int16_t room_num = Room_FindByPos(pos.x, pos.y, pos.z);
    if (room_num != NO_ROOM_NEG && item->room_num != room_num) {
        Item_NewRoom(item_num, room_num);
    }

    Lara_Animate(item);
}

void __cdecl CutscenePlayer1_Initialise(const int16_t item_num)
{
    OBJECT *const obj = &g_Objects[O_LARA];
    obj->draw_routine = Lara_Draw;
    obj->control = Lara_Control_Cutscene;

    Item_AddActive(item_num);
    ITEM *const item = &g_Items[item_num];
    g_Camera.pos.pos.x = item->pos.x;
    g_Camera.pos.pos.y = item->pos.y;
    g_Camera.pos.pos.z = item->pos.z;
    g_Camera.target_angle = 0;
    g_Camera.pos.room_num = item->room_num;
    g_OriginalRoom = g_Camera.pos.room_num;

    item->rot.y = 0;
    item->dynamic_light = 0;
    item->goal_anim_state = 0;
    item->current_anim_state = 0;
    item->frame_num = 0;
    item->anim_num = 0;

    g_Lara.hit_direction = -1;
}

void __cdecl CutscenePlayerGen_Initialise(const int16_t item_num)
{
    Item_AddActive(item_num);
    ITEM *const item = &g_Items[item_num];
    item->rot.y = 0;
    item->dynamic_light = 0;
}

int32_t __cdecl Level_Initialise(
    const int32_t level_num, const GAMEFLOW_LEVEL_TYPE level_type)
{
    g_GameInfo.current_level.num = level_num;
    g_GameInfo.current_level.type = level_type;

    if (level_type != GFL_TITLE && level_type != GFL_CUTSCENE) {
        g_CurrentLevel = level_num;
    }
    g_IsDemoLevelType = level_type == GFL_DEMO;
    InitialiseGameFlags();
    g_Lara.item_num = NO_ITEM;

    bool result;
    if (level_type == GFL_TITLE) {
        result = S_LoadLevelFile(g_GF_TitleFileNames[0], level_num, level_type);
    } else if (level_type == GFL_CUTSCENE) {
        result = S_LoadLevelFile(
            g_GF_CutsceneFileNames[level_num], level_num, level_type);
    } else {
        result = S_LoadLevelFile(
            g_GF_LevelFileNames[level_num], level_num, level_type);
    }
    if (!result) {
        return result;
    }

    if (g_Lara.item_num != NO_ITEM) {
        Lara_Initialise(level_type);
    }
    if (level_type == GFL_NORMAL || level_type == GFL_SAVED
        || level_type == GFL_DEMO) {
        GetCarriedItems();
    }
    g_Effects = GameBuf_Alloc(MAX_EFFECTS * sizeof(FX), GBUF_EFFECTS_ARRAY);
    Effect_InitialiseArray();
    LOT_InitialiseArray();
    Inv_InitColors();
    Overlay_HideGameInfo();
    Overlay_InitialisePickUpDisplay();
    g_HealthBarTimer = 100;
    Sound_StopAllSamples();
    if (level_type == GFL_SAVED) {
        ExtractSaveGameInfo();
    } else if (level_type == GFL_NORMAL) {
        GF_ModifyInventory(g_CurrentLevel, 0);
    }

    if (g_Objects[O_FINAL_LEVEL_COUNTER].loaded) {
        InitialiseFinalLevel();
    }

    if (level_type == GFL_NORMAL || level_type == GFL_SAVED
        || level_type == GFL_DEMO) {
        if (g_GF_MusicTracks[0]) {
            Music_Play(g_GF_MusicTracks[0], MPM_LOOPED);
        }
    }
    g_IsAssaultTimerActive = 0;
    g_IsAssaultTimerDisplay = 0;
    g_Camera.underwater = 0;
    return true;
}

int32_t __cdecl Misc_Move3DPosTo3DPos(
    PHD_3DPOS *const src_pos, const PHD_3DPOS *const dst_pos,
    const int32_t velocity, const PHD_ANGLE ang_add)
{
    // TODO: this function's only usage is in Lara_MovePosition. inline it
    const XYZ_32 dpos = {
        .x = dst_pos->pos.x - src_pos->pos.x,
        .y = dst_pos->pos.y - src_pos->pos.y,
        .z = dst_pos->pos.z - src_pos->pos.z,
    };
    const int32_t dist = XYZ_32_GetDistance0(&dpos);
    if (velocity >= dist) {
        src_pos->pos.x = dst_pos->pos.x;
        src_pos->pos.y = dst_pos->pos.y;
        src_pos->pos.z = dst_pos->pos.z;
    } else {
        src_pos->pos.x += velocity * dpos.x / dist;
        src_pos->pos.y += velocity * dpos.y / dist;
        src_pos->pos.z += velocity * dpos.z / dist;
    }

#define ADJUST_ROT(source, target, rot)                                        \
    do {                                                                       \
        if ((PHD_ANGLE)(target - source) > rot) {                              \
            source += rot;                                                     \
        } else if ((PHD_ANGLE)(target - source) < -rot) {                      \
            source -= rot;                                                     \
        } else {                                                               \
            source = target;                                                   \
        }                                                                      \
    } while (0)

    ADJUST_ROT(src_pos->rot.x, dst_pos->rot.x, ang_add);
    ADJUST_ROT(src_pos->rot.y, dst_pos->rot.y, ang_add);
    ADJUST_ROT(src_pos->rot.z, dst_pos->rot.z, ang_add);

    // clang-format off
    return (
        src_pos->pos.x == dst_pos->pos.x &&
        src_pos->pos.y == dst_pos->pos.y &&
        src_pos->pos.z == dst_pos->pos.z &&
        src_pos->rot.x == dst_pos->rot.x &&
        src_pos->rot.y == dst_pos->rot.y &&
        src_pos->rot.z == dst_pos->rot.z
    );
    // clang-format on
}

int32_t __cdecl LevelCompleteSequence(void)
{
    return GFD_EXIT_TO_TITLE;
}

void __cdecl S_Wait(int32_t frames, const BOOL input_check)
{
    if (input_check) {
        while (frames > 0) {
            Shell_ProcessEvents();
            Input_Update();
            Shell_ProcessInput();

            if (!g_Input.any) {
                break;
            }
            frames -= Clock_WaitTick() * TICKS_PER_FRAME;

            if (g_IsGameToExit) {
                break;
            }
        }
    }

    while (frames > 0) {
        Shell_ProcessEvents();
        Input_Update();
        Shell_ProcessInput();
        if (input_check && (g_InputDB.menu_back || g_InputDB.menu_confirm)) {
            break;
        }

        Output_BeginScene();
        Output_DrawBackground();
        Console_Draw();
        Text_Draw();
        Output_DrawPolyList();
        frames -= Output_EndScene() * TICKS_PER_FRAME;

        if (g_IsGameToExit) {
            break;
        }
    }
}

BOOL __cdecl S_InitialiseSystem(void)
{
    Random_Seed();
    Output_CalculateWibbleTable();
    return 1;
}

void __cdecl DisplayCredits(void)
{
    S_UnloadLevelFile();
    if (!Level_Initialise(0, 0)) {
        return;
    }

    Music_Play(MX_SKIDOO_THEME, MPM_ALWAYS);

    FADER fader;
    for (int32_t i = 0; i < 8; i++) {
        char file_name[60];
        sprintf(file_name, "data/credit0%d.pcx", i + 1);

        Fader_InitBlackToTransparent(&fader, FRAMES_PER_SECOND / 2);
        Output_LoadBackgroundFromFile(file_name);

        while (Fader_Control(&fader)) {
            Output_BeginScene();
            Output_DrawBackground();
            Output_DrawPolyList();
            Output_DrawBlackRectangle(fader.current.value);
            Console_Draw();
            Text_Draw();
            Output_DrawPolyList();
            Output_EndScene();
        }

        if (!g_InputDB.menu_confirm && !g_InputDB.menu_back) {
            S_Wait(15 * FRAMES_PER_SECOND, true);
        }

        Fader_InitTransparentToBlack(&fader, FRAMES_PER_SECOND / 2);
        while (Fader_Control(&fader)) {
            Output_BeginScene();
            Output_DrawBackground();
            Output_DrawPolyList();
            Output_DrawBlackRectangle(fader.current.value);
            Console_Draw();
            Text_Draw();
            Output_DrawPolyList();
            Output_EndScene();
        }

        Output_UnloadBackground();
        if (g_IsGameToExit) {
            break;
        }
    }
}

void __cdecl IncreaseScreenSize(void)
{
    if (g_Config.rendering.sizer < 1.0) {
        g_Config.rendering.sizer += 0.08;
        CLAMPG(g_Config.rendering.sizer, 1.0);
        Viewport_Reset();
    }
}

void __cdecl DecreaseScreenSize(void)
{
    if (g_Config.rendering.sizer > 0.44) {
        g_Config.rendering.sizer -= 0.08;
        CLAMPL(g_Config.rendering.sizer, 0.44);
        Viewport_Reset();
    }
}

BOOL __cdecl S_LoadLevelFile(
    const char *const file_name, const int32_t level_num,
    const GAMEFLOW_LEVEL_TYPE level_type)
{
    S_UnloadLevelFile();
    return Level_Load(file_name, level_num);
}

void __cdecl S_UnloadLevelFile(void)
{
    strcpy(g_LevelFileName, "");
    memset(g_TexturePageBuffer8, 0, sizeof(uint8_t *) * MAX_TEXTURE_PAGES);
    memset(g_TexturePageBuffer16, 0, sizeof(uint16_t *) * MAX_TEXTURE_PAGES);
    g_TextureInfoCount = 0;
}

void __cdecl GetValidLevelsList(REQUEST_INFO *const req)
{
    Requester_RemoveAllItems(req);
    for (int32_t i = 1; i < g_GameFlow.num_levels; i++) {
        Requester_AddItem(req, g_GF_LevelNames[i], 0, NULL, 0);
    }
}
