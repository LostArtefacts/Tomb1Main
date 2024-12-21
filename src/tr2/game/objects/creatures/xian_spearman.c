#include "game/objects/creatures/xian_spearman.h"

#include "game/creature.h"
#include "game/lara/control.h"
#include "game/objects/common.h"
#include "game/objects/creatures/xian_common.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/debug.h>

// clang-format off
#define XIAN_SPEARMAN_HITPOINTS    100
#define XIAN_SPEARMAN_RADIUS       (WALL_L / 5) // = 204
#define XIAN_SPEARMAN_TOUCH_L_BITS 0b00000000'00001000'00000000 // = 0x00800
#define XIAN_SPEARMAN_TOUCH_R_BITS 0b00000100'00000000'00000000 // = 0x40000
// clang-format on

static const BITE m_XianSpearmanLeftSpear = {
    .pos = { .x = 0, .y = 0, .z = 920 },
    .mesh_num = 11,
};

static const BITE m_XianSpearmanRightSpear = {
    .pos = { .x = 0, .y = 0, .z = 920 },
    .mesh_num = 18,
};

void XianSpearman_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_XIAN_SPEARMAN];
    if (!obj->loaded) {
        return;
    }

    ASSERT(g_Objects[O_XIAN_SPEARMAN_STATUE].loaded);
    obj->initialise = XianKnight_Initialise;
    obj->draw_routine = XianWarrior_Draw;
    obj->control = XianSpearman_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = XIAN_SPEARMAN_HITPOINTS;
    obj->radius = XIAN_SPEARMAN_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 0;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx + 6 * 4] |= BF_ROT_Y;
    g_AnimBones[obj->bone_idx + 12 * 4] |= BF_ROT_Y;
}

void __cdecl XianSpearman_DoDamage(
    const ITEM *const item, CREATURE *const creature, const int32_t damage)
{
    if ((creature->flags & 1) == 0
        && (item->touch_bits & XIAN_SPEARMAN_TOUCH_R_BITS) != 0) {
        Lara_TakeDamage(damage, true);
        Creature_Effect(item, &m_XianSpearmanRightSpear, DoBloodSplat);
        creature->flags |= 1;
        Sound_Effect(SFX_CRUNCH_2, &item->pos, SPM_NORMAL);
    }

    if ((creature->flags & 2) == 0
        && (item->touch_bits & XIAN_SPEARMAN_TOUCH_L_BITS) != 0) {
        Lara_TakeDamage(damage, true);
        Creature_Effect(item, &m_XianSpearmanLeftSpear, DoBloodSplat);
        creature->flags |= 2;
        Sound_Effect(SFX_CRUNCH_2, &item->pos, SPM_NORMAL);
    }
}
