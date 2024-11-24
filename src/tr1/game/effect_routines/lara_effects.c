#include "game/effect_routines/lara_effects.h"

#include "game/items.h"
#include "game/viewport.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#include <stdint.h>

void FX_LaraNormal(ITEM *item)
{
    item->current_anim_state = LS_STOP;
    item->goal_anim_state = LS_STOP;
    Item_SwitchToAnim(item, LA_STOP, 0);
    g_Camera.type = CAM_CHASE;
    Viewport_SetFOV(-1);
}

void FX_LaraHandsFree(ITEM *item)
{
    g_Lara.gun_status = LGS_ARMLESS;
}

void FX_LaraDrawRightGun(ITEM *item)
{
    Object_SwapMesh(item->object_id, O_PISTOL_ANIM, LM_THIGH_R);
    Object_SwapMesh(item->object_id, O_PISTOL_ANIM, LM_HAND_R);
}
