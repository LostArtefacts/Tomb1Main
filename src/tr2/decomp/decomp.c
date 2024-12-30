#include "decomp/decomp.h"

#include "config.h"
#include "decomp/savegame.h"
#include "decomp/stats.h"
#include "game/clock.h"
#include "game/collide.h"
#include "game/console/common.h"
#include "game/effects.h"
#include "game/game.h"
#include "game/gameflow.h"
#include "game/gameflow/gameflow_new.h"
#include "game/input.h"
#include "game/inventory.h"
#include "game/items.h"
#include "game/lara/control.h"
#include "game/lara/draw.h"
#include "game/level.h"
#include "game/lot.h"
#include "game/math.h"
#include "game/music.h"
#include "game/objects/vars.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/phase.h"
#include "game/random.h"
#include "game/requester.h"
#include "game/room.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/text.h"
#include "game/viewport.h"
#include "global/const.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/engine/image.h>
#include <libtrx/filesystem.h>
#include <libtrx/game/gamebuf.h>
#include <libtrx/game/ui/common.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>
#include <libtrx/virtual_file.h>

#include <SDL2/SDL.h>
#include <stdio.h>

// TODO: delegate these constants to individual vehicle code
#define VEHICLE_MIN_BOUNCE 50
#define VEHICLE_MAX_KICK -80

// TODO: delegate this enum to individual vehicle code
typedef enum {
    LA_VEHICLE_HIT_LEFT = 11,
    LA_VEHICLE_HIT_RIGHT = 12,
    LA_VEHICLE_HIT_FRONT = 13,
    LA_VEHICLE_HIT_BACK = 14,
} LARA_ANIM_VEHICLE;

int16_t TitleSequence(void)
{
    GF_N_LoadStrings(-1);

    if (!Level_Initialise(0, GFL_TITLE)) {
        return GFD_EXIT_GAME;
    }

    if (g_GameFlow.title_track) {
        Music_Play(g_GameFlow.title_track, MPM_LOOPED);
    }

    return GF_ShowInventory(INV_TITLE_MODE);
}

void Game_SetCutsceneTrack(const int32_t track)
{
    g_CineTrackID = track;
}

void CutscenePlayer_Control(const int16_t item_num)
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
        Output_AddDynamicLight(pos.x, pos.y, pos.z, 12, 11);
    }

    Item_Animate(item);
}

void Lara_Control_Cutscene(const int16_t item_num)
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

void CutscenePlayer1_Initialise(const int16_t item_num)
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

    item->rot.y = 0;
    item->dynamic_light = 0;
    item->goal_anim_state = 0;
    item->current_anim_state = 0;
    item->frame_num = 0;
    item->anim_num = 0;

    g_Lara.hit_direction = -1;
}

void CutscenePlayerGen_Initialise(const int16_t item_num)
{
    Item_AddActive(item_num);
    ITEM *const item = &g_Items[item_num];
    item->rot.y = 0;
    item->dynamic_light = 0;
}

int32_t Level_Initialise(
    const int32_t level_num, const GAMEFLOW_LEVEL_TYPE level_type)
{
    g_GameInfo.current_level.num = level_num;
    g_GameInfo.current_level.type = level_type;

    if (level_type != GFL_TITLE && level_type != GFL_CUTSCENE) {
        g_CurrentLevel = level_num;
    }
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

    Effect_InitialiseArray();
    LOT_InitialiseArray();
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
    g_IsAssaultTimerActive = false;
    g_IsAssaultTimerDisplay = false;
    g_Camera.underwater = 0;
    return true;
}

int32_t Misc_Move3DPosTo3DPos(
    PHD_3DPOS *const src_pos, const PHD_3DPOS *const dst_pos,
    const int32_t velocity, const int16_t ang_add)
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
        if ((int16_t)(target - source) > rot) {                                \
            source += rot;                                                     \
        } else if ((int16_t)(target - source) < -rot) {                        \
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

int32_t LevelCompleteSequence(void)
{
    return GFD_EXIT_TO_TITLE;
}

void S_Wait(int32_t frames, const bool input_check)
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
        frames -= Output_EndScene(true) * TICKS_PER_FRAME;

        if (g_IsGameToExit) {
            break;
        }
    }
}

GAME_FLOW_DIR DisplayCredits(void)
{
    S_UnloadLevelFile();
    if (!Level_Initialise(0, GFL_TITLE)) {
        return GFD_EXIT_TO_TITLE;
    }

    Music_Play(MX_SKIDOO_THEME, MPM_ALWAYS);

    for (int32_t i = 0; i < 8; i++) {
        char file_name[60];
        sprintf(file_name, "data/credit0%d.pcx", i + 1);

        PHASE *const phase = Phase_Picture_Create((PHASE_PICTURE_ARGS) {
            .file_name = file_name,
            .display_time = 15 * FRAMES_PER_SECOND,
            .fade_in_time = FRAMES_PER_SECOND / 2,
            .fade_out_time = FRAMES_PER_SECOND / 2,
        });
        const GAME_FLOW_DIR dir = PhaseExecutor_Run(phase);
        Phase_Picture_Destroy(phase);

        if (dir != (GAME_FLOW_DIR)-1) {
            return dir;
        }
    }

    {
        PHASE *const phase = Phase_Stats_Create((PHASE_STATS_ARGS) {
            .show_final_stats = true,
            .fade_in_time = FRAMES_PER_SECOND / 2,
            .fade_out_time = FRAMES_PER_SECOND / 2,
        });
        const GAME_FLOW_DIR dir = PhaseExecutor_Run(phase);

        Phase_Stats_Destroy(phase);
        if (dir != (GAME_FLOW_DIR)-1) {
            return dir;
        }
    }

    return GFD_EXIT_TO_TITLE;
}

void IncreaseScreenSize(void)
{
    if (g_Config.rendering.sizer < 1.0) {
        g_Config.rendering.sizer += 0.08;
        CLAMPG(g_Config.rendering.sizer, 1.0);
        Viewport_Reset();
    }
}

void DecreaseScreenSize(void)
{
    if (g_Config.rendering.sizer > 0.44) {
        g_Config.rendering.sizer -= 0.08;
        CLAMPL(g_Config.rendering.sizer, 0.44);
        Viewport_Reset();
    }
}

bool S_LoadLevelFile(
    const char *const file_name, const int32_t level_num,
    const GAMEFLOW_LEVEL_TYPE level_type)
{
    S_UnloadLevelFile();
    return Level_Load(file_name, level_num);
}

void S_UnloadLevelFile(void)
{
    strcpy(g_LevelFileName, "");
    memset(g_TexturePageBuffer8, 0, sizeof(uint8_t *) * MAX_TEXTURE_PAGES);
    memset(g_TexturePageBuffer16, 0, sizeof(uint16_t *) * MAX_TEXTURE_PAGES);
    g_TextureInfoCount = 0;
}

void GetValidLevelsList(REQUEST_INFO *const req)
{
    Requester_RemoveAllItems(req);
    for (int32_t i = 1; i < g_GameFlow.num_levels; i++) {
        Requester_AddItem(req, g_GF_LevelNames[i], 0, NULL, 0);
    }
}

void InitialiseGameFlags(void)
{
    for (int32_t i = 0; i < MAX_CD_TRACKS; i++) {
        g_MusicTrackFlags[i] = 0;
    }
    for (GAME_OBJECT_ID object_id = 0; object_id < O_NUMBER_OF; object_id++) {
        g_Objects[object_id].loaded = 0;
    }

    g_FlipStatus = false;
    for (int32_t i = 0; i < MAX_FLIP_MAPS; i++) {
        g_FlipMaps[i] = 0;
    }

    g_SunsetTimer = 0;
    g_LevelComplete = false;
    g_FlipEffect = -1;
    g_DetonateAllMines = false;
    g_IsMonkAngry = false;
}

void InitialiseLevelFlags(void)
{
    g_SaveGame.current_stats.timer = 0;
    g_SaveGame.current_stats.kills = 0;
    g_SaveGame.current_stats.distance = 0;
    g_SaveGame.current_stats.ammo_hits = 0;
    g_SaveGame.current_stats.ammo_used = 0;
    g_SaveGame.current_stats.medipacks = 0;
    g_SaveGame.current_stats.secrets_bitmap = 0;
}

void GetCarriedItems(void)
{
    for (int32_t item_num = 0; item_num < g_LevelItemCount; item_num++) {
        ITEM *const item = Item_Get(item_num);
        if (!g_Objects[item->object_id].intelligent) {
            continue;
        }
        item->carried_item = NO_ITEM;

        const ROOM *const room = Room_Get(item->room_num);
        int16_t pickup_item_num = room->item_num;
        do {
            ITEM *const pickup_item = Item_Get(pickup_item_num);

            if (pickup_item->pos.x == item->pos.x
                && pickup_item->pos.y == item->pos.y
                && pickup_item->pos.z == item->pos.z
                && Object_IsObjectType(
                    pickup_item->object_id, g_PickupObjects)) {
                pickup_item->carried_item = item->carried_item;
                item->carried_item = pickup_item_num;
                Item_RemoveDrawn(pickup_item_num);
                pickup_item->room_num = NO_ROOM;
            }

            pickup_item_num = pickup_item->next_item;
        } while (pickup_item_num != NO_ITEM);
    }
}

int32_t DoShift(
    ITEM *const vehicle, const XYZ_32 *const pos, const XYZ_32 *const old)
{
    int32_t x = pos->x >> WALL_SHIFT;
    int32_t z = pos->z >> WALL_SHIFT;
    const int32_t old_x = old->x >> WALL_SHIFT;
    const int32_t old_z = old->z >> WALL_SHIFT;
    const int32_t shift_x = pos->x & (WALL_L - 1);
    const int32_t shift_z = pos->z & (WALL_L - 1);

    if (x == old_x) {
        if (z == old_z) {
            vehicle->pos.x += old->x - pos->x;
            vehicle->pos.z += old->z - pos->z;
        } else if (z > old_z) {
            vehicle->pos.z -= shift_z + 1;
            return pos->x - vehicle->pos.x;
        } else {
            vehicle->pos.z += WALL_L - shift_z;
            return vehicle->pos.x - pos->x;
        }
    } else if (z == old_z) {
        if (x > old_x) {
            vehicle->pos.x -= shift_x + 1;
            return vehicle->pos.z - pos->z;
        } else {
            vehicle->pos.x += WALL_L - shift_x;
            return pos->z - vehicle->pos.z;
        }
    } else {
        int16_t room_num;
        const SECTOR *sector;
        int32_t height;

        x = 0;
        z = 0;

        room_num = vehicle->room_num;
        sector = Room_GetSector(old->x, pos->y, pos->z, &room_num);
        height = Room_GetHeight(sector, old->x, pos->y, pos->z);
        if (height < old->y - STEP_L) {
            if (pos->z > old->z) {
                z = -shift_z - 1;
            } else {
                z = WALL_L - shift_z;
            }
        }

        room_num = vehicle->room_num;
        sector = Room_GetSector(pos->x, pos->y, old->z, &room_num);
        height = Room_GetHeight(sector, pos->x, pos->y, old->z);
        if (height < old->y - STEP_L) {
            if (pos->x > old->x) {
                x = -shift_x - 1;
            } else {
                x = WALL_L - shift_x;
            }
        }

        if (x != 0 && z != 0) {
            vehicle->pos.x += x;
            vehicle->pos.z += z;
        } else if (z != 0) {
            vehicle->pos.z += z;
            if (z > 0) {
                return vehicle->pos.x - pos->x;
            } else {
                return pos->x - vehicle->pos.x;
            }
        } else if (x != 0) {
            vehicle->pos.x += x;
            if (x > 0) {
                return pos->z - vehicle->pos.z;
            } else {
                return vehicle->pos.z - pos->z;
            }
        } else {
            vehicle->pos.x += old->x - pos->x;
            vehicle->pos.z += old->z - pos->z;
        }
    }

    return 0;
}

int32_t DoDynamics(
    const int32_t height, const int32_t fall_speed, int32_t *const out_y)
{
    if (height > *out_y) {
        *out_y += fall_speed;
        if (*out_y > height - VEHICLE_MIN_BOUNCE) {
            *out_y = height;
            return 0;
        }
        return fall_speed + GRAVITY;
    }

    int32_t kick = 4 * (height - *out_y);
    CLAMPL(kick, VEHICLE_MAX_KICK);
    CLAMPG(*out_y, height);
    return fall_speed + ((kick - fall_speed) >> 3);
}

int32_t GetCollisionAnim(const ITEM *const vehicle, XYZ_32 *const moved)
{
    moved->x = vehicle->pos.x - moved->x;
    moved->z = vehicle->pos.z - moved->z;

    if (moved->x != 0 || moved->z != 0) {
        const int32_t c = Math_Cos(vehicle->rot.y);
        const int32_t s = Math_Sin(vehicle->rot.y);
        const int32_t front = (moved->x * s + moved->z * c) >> W2V_SHIFT;
        const int32_t side = (moved->x * c - moved->z * s) >> W2V_SHIFT;
        if (ABS(front) > ABS(side)) {
            if (front > 0) {
                return LA_VEHICLE_HIT_BACK;
            } else {
                return LA_VEHICLE_HIT_FRONT;
            }
        } else {
            if (side > 0) {
                return LA_VEHICLE_HIT_LEFT;
            } else {
                return LA_VEHICLE_HIT_RIGHT;
            }
        }
    }

    return 0;
}

void InitialiseFinalLevel(void)
{
    g_FinalBossActive = 0;
    g_FinalBossCount = 0;
    g_FinalLevelCount = 0;

    for (int32_t item_num = 0; item_num < g_LevelItemCount; item_num++) {
        const ITEM *const item = Item_Get(item_num);

        switch (item->object_id) {
        case O_DOG:
        case O_CULT_1:
        case O_WORKER_3:
            g_FinalLevelCount++;
            break;

        case O_CULT_3:
            g_FinalBossItem[g_FinalBossCount] = item_num;
            g_FinalBossCount++;
            if (item->status == IS_ACTIVE) {
                g_FinalBossActive = 1;
            } else if (item->status == IS_DEACTIVATED) {
                g_FinalBossActive = 2;
            }
            break;

        default:
            break;
        }
    }
}
