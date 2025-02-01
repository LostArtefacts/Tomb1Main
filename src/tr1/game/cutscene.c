#include "game/cutscene.h"

#include "game/camera.h"
#include "game/effects.h"
#include "game/game.h"
#include "game/game_flow.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/items.h"
#include "game/lara/common.h"
#include "game/lara/hair.h"
#include "game/level.h"
#include "game/music.h"
#include "game/output.h"
#include "game/shell.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/memory.h>

static void M_InitialiseLara(const GF_LEVEL *level);

static void M_InitialiseLara(const GF_LEVEL *const level)
{
    const GAME_OBJECT_ID lara_type = level->lara_type;
    Lara_Hair_SetLaraType(lara_type);
    if (!Lara_Hair_IsActive()) {
        return;
    }

    if (lara_type == O_LARA) {
        return;
    }

    int16_t lara_item_num = NO_ITEM;
    for (int32_t i = 0; i < Item_GetTotalCount(); i++) {
        if (g_Items[i].object_id == lara_type) {
            lara_item_num = i;
            break;
        }
    }

    if (lara_item_num == NO_ITEM) {
        return;
    }

    Lara_InitialiseLoad(lara_item_num);
    Lara_Initialise(level);

    Item_SwitchToObjAnim(g_LaraItem, 0, 0, lara_type);
    const ANIM *const cut_anim = Item_GetAnim(g_LaraItem);
    g_LaraItem->current_anim_state = g_LaraItem->goal_anim_state =
        g_LaraItem->required_anim_state = cut_anim->current_anim_state;
}

bool Cutscene_Start(const int32_t level_num)
{
    const GF_LEVEL *const level = GF_GetLevel(GFLT_CUTSCENES, level_num);
    ASSERT(GF_GetCurrentLevel() == level);

    M_InitialiseLara(level);

    for (int16_t room_num = 0; room_num < g_RoomCount; room_num++) {
        if (g_RoomInfo[room_num].flipped_room >= 0) {
            g_RoomInfo[g_RoomInfo[room_num].flipped_room].bound_active = 1;
        }
    }

    g_RoomsToDrawCount = 0;
    for (int16_t room_num = 0; room_num < g_RoomCount; room_num++) {
        if (!g_RoomInfo[room_num].bound_active) {
            if (g_RoomsToDrawCount + 1 < MAX_ROOMS_TO_DRAW) {
                g_RoomsToDraw[g_RoomsToDrawCount++] = room_num;
            }
        }
    }

    g_CineFrame = 0;
    return true;
}

void Cutscene_End(void)
{
    Music_Stop();
    Sound_StopAll();
}

GF_COMMAND Cutscene_Control(void)
{
    Interpolation_Remember();

    Input_Update();
    Shell_ProcessInput();
    if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
        return (GF_COMMAND) { .action = GF_LEVEL_COMPLETE };
    } else if (g_InputDB.pause) {
        const GF_COMMAND gf_cmd = GF_PauseGame();
        if (gf_cmd.action != GF_NOOP) {
            return gf_cmd;
        }
    } else if (g_InputDB.toggle_photo_mode) {
        const GF_COMMAND gf_cmd = GF_EnterPhotoMode();
        if (gf_cmd.action != GF_NOOP) {
            return gf_cmd;
        }
    }

    Output_ResetDynamicLights();

    Item_Control();
    Effect_Control();
    Lara_Hair_Control();
    Camera_UpdateCutscene();
    Output_AnimateTextures(1);

    g_CineFrame++;
    if (g_CineFrame >= g_NumCineFrames - 1) {
        g_CineFrame = g_NumCineFrames - 2;
        return (GF_COMMAND) { .action = GF_LEVEL_COMPLETE };
    }

    return (GF_COMMAND) { .action = GF_NOOP };
}

void Cutscene_Draw(void)
{
    Game_Draw(true);
}
