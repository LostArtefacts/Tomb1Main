#include "game/objects/creatures/spider.h"

#include "game/creature.h"
#include "game/items.h"
#include "game/lara/control.h"
#include "game/lot.h"
#include "game/objects/common.h"
#include "game/random.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

// clang-format off
#define SPIDER_HITPOINTS      5
#define SPIDER_TURN           (PHD_DEGREE * 8) // = 1456
#define SPIDER_RADIUS         (WALL_L / 10) // = 102
#define SPIDER_ATTACK_2_RANGE SQUARE(WALL_L / 2) // = 262144
#define SPIDER_ATTACK_3_RANGE SQUARE(WALL_L / 5) // = 41616
#define SPIDER_DAMAGE         25
// clang-format on

typedef enum {
    // clang-format off
    SPIDER_STATE_EMPTY    = 0,
    SPIDER_STATE_STOP     = 1,
    SPIDER_STATE_WALK_1   = 2,
    SPIDER_STATE_WALK_2   = 3,
    SPIDER_STATE_ATTACK_1 = 4,
    SPIDER_STATE_ATTACK_2 = 5,
    SPIDER_STATE_ATTACK_3 = 6,
    SPIDER_STATE_DEATH    = 7,
    // clang-format on
} SPIDER_STATE;

typedef enum {
    SPIDER_ANIM_LEAP = 2,
} SPIDER_ANIM;

static const BITE m_SpiderBite = {
    .pos = { .x = 0, .y = 0, .z = 41 },
    .mesh_num = 1,
};

void Spider_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_SPIDER];
    if (!obj->loaded) {
        return;
    }

    obj->control = Spider_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = SPIDER_HITPOINTS;
    obj->radius = SPIDER_RADIUS;
    obj->shadow_size = UNIT_SHADOW / 2;

    obj->intelligent = 1;
    obj->save_anim = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
}

void __cdecl Spider_Leap(const int16_t item_num, const int16_t angle)
{
    ITEM *const item = Item_Get(item_num);
    const XYZ_32 old_pos = item->pos;
    const int16_t old_room_num = item->room_num;

    Creature_Animate(item_num, angle, 0);
    if (item->pos.y > old_pos.y - STEP_L * 3 / 2) {
        return;
    }

    item->pos = old_pos;
    if (item->room_num != old_room_num) {
        Item_NewRoom(item_num, old_room_num);
    }
    item->anim_num = g_Objects[O_SPIDER].anim_idx + SPIDER_ANIM_LEAP;
    item->frame_num = g_Anims[item->anim_num].frame_base;
    item->current_anim_state = SPIDER_STATE_ATTACK_2;
    Creature_Animate(item_num, angle, 0);
}

void __cdecl Spider_Control(const int16_t item_num)
{
    if (!Creature_Activate(item_num)) {
        return;
    }

    int16_t angle = 0;
    ITEM *const item = Item_Get(item_num);
    CREATURE *const creature = item->data;

    if (item->hit_points > 0) {
        AI_INFO info;
        Creature_AIInfo(&g_Items[item_num], &info);
        Creature_Mood(item, &info, MOOD_BORED);

        angle = Creature_Turn(item, SPIDER_TURN);

        switch (item->current_anim_state) {
        case SPIDER_STATE_STOP:
            creature->flags = 0;
            if (creature->mood == MOOD_BORED) {
                if (Random_GetControl() < 256) {
                    item->goal_anim_state = SPIDER_STATE_WALK_1;
                }
            } else if (info.ahead != 0 && item->touch_bits != 0) {
                item->goal_anim_state = SPIDER_STATE_ATTACK_1;
            } else if (creature->mood == MOOD_STALK) {
                item->goal_anim_state = SPIDER_STATE_WALK_1;
            } else if (
                creature->mood == MOOD_ESCAPE
                || creature->mood == MOOD_ATTACK) {
                item->goal_anim_state = SPIDER_STATE_WALK_2;
            }
            break;

        case SPIDER_STATE_WALK_1:
            if (creature->mood == MOOD_BORED) {
                if (Random_GetControl() < 256) {
                    item->goal_anim_state = SPIDER_STATE_STOP;
                }
            } else if (
                creature->mood == MOOD_ESCAPE
                || creature->mood == MOOD_ATTACK) {
                item->goal_anim_state = SPIDER_STATE_WALK_2;
            }
            break;

        case SPIDER_STATE_WALK_2:
            creature->flags = 0;
            if (creature->mood == MOOD_BORED || creature->mood == MOOD_STALK) {
                item->goal_anim_state = SPIDER_STATE_WALK_1;
            } else if (info.ahead != 0 && item->touch_bits != 0) {
                item->goal_anim_state = SPIDER_STATE_STOP;
            } else if (
                info.ahead != 0 && info.distance < SPIDER_ATTACK_3_RANGE) {
                item->goal_anim_state = SPIDER_STATE_ATTACK_3;
            } else if (
                info.ahead != 0 && info.distance < SPIDER_ATTACK_2_RANGE) {
                item->goal_anim_state = SPIDER_STATE_ATTACK_2;
            }
            break;

        case SPIDER_STATE_ATTACK_1:
        case SPIDER_STATE_ATTACK_2:
        case SPIDER_STATE_ATTACK_3:
            if (creature->flags == 0 && item->touch_bits != 0) {
                Creature_Effect(item, &m_SpiderBite, DoBloodSplat);
                Lara_TakeDamage(SPIDER_DAMAGE, true);
                creature->flags = 1;
            }
            break;

        default:
            break;
        }
    } else if (Effect_ExplodingDeath(item_num, -1, 0)) {
        LOT_DisableBaddieAI(item_num);
        Item_Kill(item_num);
        item->status = IS_DEACTIVATED;
        Sound_Effect(SFX_SPIDER_EXPLODE, &item->pos, SPM_NORMAL);
        return;
    }

    Spider_Leap(item_num, angle);
}
