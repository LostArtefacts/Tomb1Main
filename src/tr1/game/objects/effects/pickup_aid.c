#include "game/objects/effects/pickup_aid.h"

#include "game/effects.h"

static void M_Control(int16_t fx_num);

static void M_Control(int16_t fx_num)
{
    FX *const fx = &g_Effects[fx_num];
    fx->counter++;
    if (fx->counter == 1) {
        fx->counter = 0;
        fx->frame_num--;
        if (fx->frame_num <= Object_GetObject(fx->object_id)->nmeshes) {
            Effect_Kill(fx_num);
        }
    }
}

void PickupAid_Setup(OBJECT *const obj)
{
    obj->control = M_Control;
}

void PickupAid_Spawn(const GAME_VECTOR *const pos)
{
    const int16_t fx_num = Effect_Create(pos->room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->pos.x = pos->x;
        fx->pos.y = pos->y;
        fx->pos.z = pos->z;
        fx->counter = 0;
        fx->object_id = O_PICKUP_AID;
        fx->frame_num = 0;
    }
}
