#include "game/objects/creatures/bird_guardian.h"

#include "game/creature.h"
#include "game/lara/control.h"
#include "game/objects/common.h"
#include "game/random.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

// clang-format off
#define BIRD_GUARDIAN_HITPOINTS      200
#define BIRD_GUARDIAN_TOUCH_BITS_L   0b00001100'00000000'00000000 // = 0x0C0000
#define BIRD_GUARDIAN_TOUCH_BITS_R   0b01100000'00000000'00000000 // = 0x600000
#define BIRD_GUARDIAN_RADIUS         (WALL_L / 3) // = 341
#define BIRD_GUARDIAN_WALK_TURN      (PHD_DEGREE * 4) // = 728
#define BIRD_GUARDIAN_ATTACK_1_RANGE SQUARE(WALL_L) // = 1048576
#define BIRD_GUARDIAN_ATTACK_2_RANGE SQUARE(WALL_L * 2) // = 4194304
#define BIRD_GUARDIAN_PUNCH_DAMAGE   200
// clang-format on

typedef enum {
    // clang-format off
    BIRD_GUARDIAN_STATE_EMPTY   = 0,
    BIRD_GUARDIAN_STATE_WAIT    = 1,
    BIRD_GUARDIAN_STATE_WALK    = 2,
    BIRD_GUARDIAN_STATE_AIM_1   = 3,
    BIRD_GUARDIAN_STATE_PUNCH_1 = 4,
    BIRD_GUARDIAN_STATE_AIM_2   = 5,
    BIRD_GUARDIAN_STATE_PUNCH_2 = 6,
    BIRD_GUARDIAN_STATE_PUNCH_R = 7,
    BIRD_GUARDIAN_STATE_WAIT_2  = 8,
    BIRD_GUARDIAN_STATE_DEATH   = 9,
    BIRD_GUARDIAN_STATE_AIM_3   = 10,
    BIRD_GUARDIAN_STATE_PUNCH_3 = 11,
    // clang-format on
} BIRD_GUARDIAN_STATE;

typedef enum {
    BIRD_GUARDIAN_ANIM_DEATH = 20,
} BIRD_GUARDIAN_ANIM;

static const BITE m_BirdGuardianBiteL = {
    .pos = { .x = 0, .y = 224, .z = 0, },
    .mesh_num = 19,
};

static const BITE m_BirdGuardianBiteR = {
    .pos = { .x = 0, .y = 224, .z = 0, },
    .mesh_num = 22,
};

void BirdGuardian_Setup(void)
{
    OBJECT *const obj = &g_Objects[O_BIRD_GUARDIAN];
    if (!obj->loaded) {
        return;
    }

    obj->control = BirdGuardian_Control;
    obj->collision = Creature_Collision;

    obj->hit_points = BIRD_GUARDIAN_HITPOINTS;
    obj->radius = BIRD_GUARDIAN_RADIUS;

    obj->intelligent = 1;
    obj->save_position = 1;
    obj->save_hitpoints = 1;
    obj->save_flags = 1;
    obj->save_anim = 1;

    g_AnimBones[obj->bone_idx + 14 * 4] |= BF_ROT_Y;
}

void __cdecl BirdGuardian_Control(const int16_t item_num)
{
    if (!Creature_Activate(item_num)) {
        return;
    }

    ITEM *const item = Item_Get(item_num);
    CREATURE *const creature = item->data;

    int16_t head = 0;
    int16_t angle = 0;

    if (item->hit_points > 0) {
        AI_INFO info;
        Creature_AIInfo(item, &info);
        if (info.ahead != 0) {
            head = info.angle;
        }

        Creature_Mood(item, &info, MOOD_ATTACK);
        angle = Creature_Turn(item, creature->maximum_turn);

        switch (item->current_anim_state) {
        case BIRD_GUARDIAN_STATE_WAIT:
            creature->maximum_turn = 0;
            if (info.ahead != 0
                && info.distance < BIRD_GUARDIAN_ATTACK_1_RANGE) {
                if (Random_GetControl() < 0x4000) {
                    item->goal_anim_state = BIRD_GUARDIAN_STATE_AIM_1;
                } else {
                    item->goal_anim_state = BIRD_GUARDIAN_STATE_AIM_3;
                }
            } else if (info.ahead != 0 && creature->mood == MOOD_BORED) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WAIT_2;
            } else if (info.ahead != 0 && creature->mood == MOOD_STALK) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WAIT_2;
            } else {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WALK;
            }
            break;

        case BIRD_GUARDIAN_STATE_WAIT_2:
            if (creature->mood != MOOD_BORED) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WAIT;
            } else if (info.ahead == 0) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WAIT;
            }
            break;

        case BIRD_GUARDIAN_STATE_WALK:
            creature->maximum_turn = BIRD_GUARDIAN_WALK_TURN;
            if (info.ahead != 0
                && info.distance < BIRD_GUARDIAN_ATTACK_2_RANGE) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_AIM_2;
            } else if (info.ahead != 0 && creature->mood == MOOD_BORED) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WAIT;
            } else if (info.ahead != 0 && creature->mood == MOOD_STALK) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WAIT;
            }
            break;

        case BIRD_GUARDIAN_STATE_AIM_1:
            creature->flags = 0;
            if (info.ahead != 0
                && info.distance < BIRD_GUARDIAN_ATTACK_1_RANGE) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_PUNCH_1;
            } else {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WAIT;
            }
            break;

        case BIRD_GUARDIAN_STATE_AIM_2:
            creature->flags = 0;
            if (info.ahead != 0
                && info.distance < BIRD_GUARDIAN_ATTACK_2_RANGE) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_PUNCH_2;
            } else {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WALK;
            }
            break;

        case BIRD_GUARDIAN_STATE_AIM_3:
            creature->flags = 0;
            if (info.ahead != 0
                && info.distance < BIRD_GUARDIAN_ATTACK_1_RANGE) {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_PUNCH_3;
            } else {
                item->goal_anim_state = BIRD_GUARDIAN_STATE_WAIT;
            }
            break;

        case BIRD_GUARDIAN_STATE_PUNCH_1:
        case BIRD_GUARDIAN_STATE_PUNCH_2:
        case BIRD_GUARDIAN_STATE_PUNCH_R:
        case BIRD_GUARDIAN_STATE_PUNCH_3:
            if ((creature->flags & 1) == 0
                && (item->touch_bits & BIRD_GUARDIAN_TOUCH_BITS_R) != 0) {
                Creature_Effect(item, &m_BirdGuardianBiteR, DoBloodSplat);
                Lara_TakeDamage(BIRD_GUARDIAN_PUNCH_DAMAGE, true);
                creature->flags |= 1;
            }
            if ((creature->flags & 2) == 0
                && (item->touch_bits & BIRD_GUARDIAN_TOUCH_BITS_L) != 0) {
                Creature_Effect(item, &m_BirdGuardianBiteL, DoBloodSplat);
                Lara_TakeDamage(BIRD_GUARDIAN_PUNCH_DAMAGE, true);
                creature->flags |= 2;
            }
            break;

        default:
            break;
        }
    } else {
        if (item->current_anim_state != BIRD_GUARDIAN_STATE_DEATH) {
            item->anim_num =
                g_Objects[O_BIRD_GUARDIAN].anim_idx + BIRD_GUARDIAN_ANIM_DEATH;
            item->frame_num = g_Anims[item->anim_num].frame_base;
            item->current_anim_state = BIRD_GUARDIAN_STATE_DEATH;
        }

        if (item->frame_num == g_Anims[item->anim_num].frame_end) {
            g_LevelComplete = true;
        }
    }

    Creature_Head(item, head);
    Creature_Animate(item_num, angle, 0);
}
