#include "game/objects/creatures/cultist_1.h"

#include "game/creature.h"
#include "game/objects/creatures/cultist_common.h"
#include "game/random.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/debug.h>

#define CULTIST_1_HITPOINTS 25

void Cultist1_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_CULT_1];
    if (!obj->loaded) {
        return;
    }

    obj->initialise = Cultist1_Initialise;
    obj->control = Cultist1_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = CULTIST_1_HITPOINTS;
    obj->radius = CULTIST_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 50;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx] |= BF_ROT_Y;
}

void Cultist1A_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_CULT_1A];
    if (!obj->loaded) {
        return;
    }

    ASSERT(g_Objects[O_CULT_1].loaded);
    obj->frame_base = g_Objects[O_CULT_1].frame_base;
    obj->anim_idx = g_Objects[O_CULT_1].anim_idx;

    obj->initialise = Cultist1_Initialise;
    obj->control = Cultist1_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = CULTIST_1_HITPOINTS;
    obj->radius = CULTIST_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 50;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx] |= BF_ROT_Y;
}

void Cultist1B_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_CULT_1B];
    if (!obj->loaded) {
        return;
    }

    ASSERT(g_Objects[O_CULT_1].loaded);
    obj->frame_base = g_Objects[O_CULT_1].frame_base;
    obj->anim_idx = g_Objects[O_CULT_1].anim_idx;

    obj->initialise = Cultist1_Initialise;
    obj->control = Cultist1_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = CULTIST_1_HITPOINTS;
    obj->radius = CULTIST_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 50;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx] |= BF_ROT_Y;
}

void __cdecl Cultist1_Initialise(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);
    if (Random_GetControl() < 0x4000) {
        item->mesh_bits &= ~0b00110000;
    }
    if (item->object_id == O_CULT_1B) {
        item->mesh_bits &= ~0b00011111'10000000'00000000;
    }
}
