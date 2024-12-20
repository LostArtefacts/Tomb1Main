#include "game/objects/creatures/diver.h"

#include "game/creature.h"
#include "game/effects.h"
#include "game/los.h"
#include "game/objects/effects/missile_common.h"
#include "game/room.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

// clang-format off
#define DIVER_SWIM_TURN     (3 * PHD_DEGREE) // = 546
#define DIVER_FRONT_ARC     PHD_45
#define DIVER_DIE_ANIM      16
#define DIVER_HITPOINTS     20
#define DIVER_RADIUS        (WALL_L / 3) // = 341
#define DIVER_HARPOON_SPEED 150
// clang-format on

typedef enum {
    // clang-format off
    DIVER_STATE_EMPTY   = 0,
    DIVER_STATE_SWIM_1  = 1,
    DIVER_STATE_SWIM_2  = 2,
    DIVER_STATE_SHOOT_1 = 3,
    DIVER_STATE_AIM_1   = 4,
    DIVER_STATE_NULL_1  = 5,
    DIVER_STATE_AIM_2   = 6,
    DIVER_STATE_SHOOT_2 = 7,
    DIVER_STATE_NULL_2  = 8,
    DIVER_STATE_DEATH   = 9,
    // clang-format on
} DIVER_STATE;

static const BITE m_DiverBite = {
    .pos = { .x = 17, .y = 164, .z = 44 },
    .mesh_num = 18,
};

static const SECTOR *M_GetRelSector(const ROOM *r, int32_t x, int32_t z);

static const SECTOR *M_GetRelSector(
    const ROOM *const r, const int32_t x, const int32_t z)
{
    const XZ_32 sector_pos = {
        .x = (x - r->pos.x) >> WALL_SHIFT,
        .z = (z - r->pos.z) >> WALL_SHIFT,
    };
    return &r->sectors[sector_pos.z + r->size.z * sector_pos.x];
}

int32_t __cdecl Diver_GetWaterSurface(
    const int32_t x, const int32_t y, const int32_t z, const int16_t room_num)
{
    const ROOM *r = Room_Get(room_num);
    const SECTOR *sector = M_GetRelSector(r, x, z);

    if ((r->flags & RF_UNDERWATER)) {
        while (sector->portal_room.sky != NO_ROOM) {
            r = Room_Get(sector->portal_room.sky);
            if (!(r->flags & RF_UNDERWATER)) {
                return sector->ceiling.height;
            }
            sector = M_GetRelSector(r, x, z);
        }
    } else {
        while (sector->portal_room.pit != NO_ROOM) {
            r = Room_Get(sector->portal_room.pit);
            if ((r->flags & RF_UNDERWATER)) {
                return sector->floor.height;
            }
            sector = M_GetRelSector(r, x, z);
        }
    }
    return NO_HEIGHT;
}

void Diver_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_DIVER];
    if (!obj->loaded) {
        return;
    }

    obj->control = Diver_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = DIVER_HITPOINTS;
    obj->radius = DIVER_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;
    obj->pivot_length = 50;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;
    obj->water_creature = 1;

    g_AnimBones[obj->bone_idx + 10 * 4] |= BF_ROT_Y;
    g_AnimBones[obj->bone_idx + 14 * 4] |= BF_ROT_Z;
}

void __cdecl Diver_Control(int16_t item_num)
{
    if (!Creature_Activate(item_num)) {
        return;
    }

    ITEM *const item = &g_Items[item_num];
    CREATURE *const creature = item->data;

    if (item->hit_points <= 0) {
        if (item->current_anim_state != DIVER_STATE_DEATH) {
            item->anim_num = g_Objects[O_DIVER].anim_idx + DIVER_DIE_ANIM;
            item->frame_num = g_Anims[item->anim_num].frame_base;
            item->current_anim_state = DIVER_STATE_DEATH;
        }
        Creature_Float(item_num);
        return;
    }

    AI_INFO info;
    Creature_AIInfo(item, &info);
    Creature_Mood(item, &info, MOOD_BORED);

    bool shoot;
    if (g_Lara.water_status == LWS_ABOVE_WATER) {
        GAME_VECTOR start;
        start.pos.x = item->pos.x;
        start.pos.y = item->pos.y - STEP_L;
        start.pos.z = item->pos.z;
        start.room_num = item->room_num;

        GAME_VECTOR target;
        target.pos.x = g_LaraItem->pos.x;
        target.pos.y = g_LaraItem->pos.y - (LARA_HEIGHT - 150);
        target.pos.z = g_LaraItem->pos.z;
        target.room_num = g_LaraItem->room_num;
        shoot = LOS_Check(&start, &target);

        if (shoot) {
            creature->target.x = g_LaraItem->pos.x;
            creature->target.y = g_LaraItem->pos.y;
            creature->target.z = g_LaraItem->pos.z;
        }

        if (info.angle < -DIVER_FRONT_ARC || info.angle > DIVER_FRONT_ARC) {
            shoot = false;
        }
    } else if (info.angle > -DIVER_FRONT_ARC && info.angle < DIVER_FRONT_ARC) {
        GAME_VECTOR start;
        start.pos.x = item->pos.x;
        start.pos.y = item->pos.y;
        start.pos.z = item->pos.z;
        start.room_num = item->room_num;

        GAME_VECTOR target;
        target.pos.x = g_LaraItem->pos.x;
        target.pos.y = g_LaraItem->pos.y;
        target.pos.z = g_LaraItem->pos.z;
        target.room_num = g_LaraItem->room_num;

        shoot = LOS_Check(&start, &target);
    } else {
        shoot = false;
    }

    int16_t head = 0;
    int16_t neck = 0;
    int16_t angle = Creature_Turn(item, creature->maximum_turn);
    int32_t water_level =
        Diver_GetWaterSurface(
            item->pos.x, item->pos.y, item->pos.z, item->room_num)
        + 512;

    switch (item->current_anim_state) {
    case DIVER_STATE_SWIM_1:
        creature->maximum_turn = DIVER_SWIM_TURN;
        if (shoot) {
            neck = -info.angle;
        }
        if (creature->target.y < water_level
            && item->pos.y < water_level + creature->lot.fly) {
            item->goal_anim_state = DIVER_STATE_SWIM_2;
        } else if (creature->mood != MOOD_ESCAPE && shoot) {
            item->goal_anim_state = DIVER_STATE_AIM_1;
        }
        break;

    case DIVER_STATE_SWIM_2:
        creature->maximum_turn = DIVER_SWIM_TURN;
        if (shoot) {
            head = info.angle;
        }
        if (creature->target.y > water_level) {
            item->goal_anim_state = DIVER_STATE_SWIM_1;
        } else if (creature->mood != MOOD_ESCAPE && shoot) {
            item->goal_anim_state = DIVER_STATE_AIM_2;
        }
        break;

    case DIVER_STATE_SHOOT_1:
        if (shoot) {
            neck = -info.angle;
        }
        if (!creature->flags) {
            Creature_Effect(item, &m_DiverBite, Diver_Harpoon);
            creature->flags = 1;
        }
        break;

    case DIVER_STATE_SHOOT_2:
        if (shoot) {
            head = info.angle;
        }
        if (!creature->flags) {
            Creature_Effect(item, &m_DiverBite, Diver_Harpoon);
            creature->flags = 1;
        }
        break;

    case DIVER_STATE_AIM_1:
        creature->flags = 0;
        if (shoot) {
            neck = -info.angle;
        }
        if (!shoot || creature->mood == MOOD_ESCAPE
            || (creature->target.y < water_level
                && item->pos.y < water_level + creature->lot.fly)) {
            item->goal_anim_state = DIVER_STATE_SWIM_1;
        } else {
            item->goal_anim_state = DIVER_STATE_SHOOT_1;
        }
        break;

    case DIVER_STATE_AIM_2:
        creature->flags = 0;
        if (shoot) {
            head = info.angle;
        }
        if (!shoot || creature->mood == MOOD_ESCAPE
            || creature->target.y > water_level) {
            item->goal_anim_state = DIVER_STATE_SWIM_2;
        } else {
            item->goal_anim_state = DIVER_STATE_SHOOT_2;
        }
        break;

    default:
        break;
    }

    Creature_Head(item, head);
    Creature_Neck(item, neck);

    Creature_Animate(item_num, angle, 0);

    switch (item->current_anim_state) {
    case DIVER_STATE_SWIM_1:
    case DIVER_STATE_AIM_1:
    case DIVER_STATE_SHOOT_1:
        Creature_Underwater(item, WALL_L / 2);
        break;

    default:
        item->pos.y = water_level - WALL_L / 2;
        break;
    }
}

int16_t __cdecl Diver_Harpoon(
    const int32_t x, const int32_t y, const int32_t z, const int16_t speed,
    const int16_t y_rot, const int16_t room_num)
{
    const int16_t fx_num = Effect_Create(room_num);
    if (fx_num != NO_ITEM) {
        FX *const fx = &g_Effects[fx_num];
        fx->pos.x = x;
        fx->pos.y = y;
        fx->pos.z = z;
        fx->room_num = room_num;
        fx->rot.x = 0;
        fx->rot.y = y_rot;
        fx->rot.z = 0;
        fx->speed = DIVER_HARPOON_SPEED;
        fx->fall_speed = 0;
        fx->frame_num = 0;
        fx->object_id = O_MISSILE_HARPOON;
        fx->shade = 3584;
        Missile_ShootAtLara(fx);
    }
    return fx_num;
}
