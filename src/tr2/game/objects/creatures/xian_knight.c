#include "game/objects/creatures/xian_knight.h"

#include "game/creature.h"
#include "game/effects.h"
#include "game/objects/common.h"
#include "game/objects/creatures/xian_common.h"
#include "game/random.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/debug.h>

#define XIAN_KNIGHT_HITPOINTS 80
#define XIAN_KNIGHT_RADIUS (WALL_L / 5) // = 204

static void M_Initialise(int16_t item_num);

static void M_Initialise(const int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    item->status = IS_INACTIVE;
    item->mesh_bits = 0;
}

void __cdecl XianKnight_SparkleTrail(const ITEM *const item)
{
    const int16_t fx_num = Effect_Create(item->room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->object_id = O_TWINKLE;
        fx->pos.x = item->pos.x + (Random_GetDraw() << 8 >> 15) - 128;
        fx->pos.y = item->pos.y + (Random_GetDraw() << 8 >> 15) - 256;
        fx->pos.z = item->pos.z + (Random_GetDraw() << 8 >> 15) - 128;
        fx->room_num = item->room_num;
        fx->counter = -30;
        fx->frame_num = 0;
    }
    Sound_Effect(SFX_WARRIOR_HOVER, &item->pos, SPM_NORMAL);
}

void XianKnight_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_XIAN_KNIGHT];
    if (!obj->loaded) {
        return;
    }

    ASSERT(g_Objects[O_XIAN_KNIGHT_STATUE].loaded);

    obj->initialise = M_Initialise;
    obj->draw_routine = XianWarrior_Draw;
    obj->control = XianKnight_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = XIAN_KNIGHT_HITPOINTS;
    obj->radius = XIAN_KNIGHT_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 0;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx + 6 * 4] |= BF_ROT_Y;
    g_AnimBones[obj->bone_idx + 16 * 4] |= BF_ROT_Y;
}
