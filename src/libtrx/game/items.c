#include "game/items.h"

#include "game/const.h"
#include "game/item_actions.h"
#include "game/lara/common.h"
#include "game/objects/common.h"
#include "game/objects/vars.h"
#include "game/rooms.h"
#include "game/sound/common.h"
#include "utils.h"

#include <stddef.h>

void Item_TakeDamage(
    ITEM *const item, const int16_t damage, const bool hit_status)
{
#if TR_VERSION == 1
    if (item->hit_points == DONT_TARGET) {
        return;
    }
#endif

    item->hit_points -= damage;
    CLAMPL(item->hit_points, 0);

    if (hit_status) {
        item->hit_status = 1;
    }
}

ITEM *Item_Find(const GAME_OBJECT_ID object_id)
{
    for (int32_t item_num = 0; item_num < Item_GetTotalCount(); item_num++) {
        ITEM *const item = Item_Get(item_num);
        if (item->object_id == object_id) {
            return item;
        }
    }

    return NULL;
}

ANIM *Item_GetAnim(const ITEM *const item)
{
    return Anim_GetAnim(item->anim_num);
}

bool Item_TestAnimEqual(const ITEM *const item, const int16_t anim_idx)
{
    const OBJECT *const object = Object_GetObject(item->object_id);
    return item->anim_num == object->anim_idx + anim_idx;
}

void Item_SwitchToAnim(
    ITEM *const item, const int16_t anim_idx, const int16_t frame)
{
    Item_SwitchToObjAnim(item, anim_idx, frame, item->object_id);
}

void Item_SwitchToObjAnim(
    ITEM *const item, const int16_t anim_idx, const int16_t frame,
    const GAME_OBJECT_ID object_id)
{
    const OBJECT *const object = Object_GetObject(object_id);
    item->anim_num = object->anim_idx + anim_idx;

    const ANIM *const anim = Item_GetAnim(item);
    if (frame < 0) {
        item->frame_num = anim->frame_end + frame + 1;
    } else {
        item->frame_num = anim->frame_base + frame;
    }
}

bool Item_TestFrameEqual(const ITEM *const item, const int16_t frame)
{
    return Anim_TestAbsFrameEqual(
        item->frame_num, Item_GetAnim(item)->frame_base + frame);
}

bool Item_TestFrameRange(
    const ITEM *const item, const int16_t start, const int16_t end)
{
    return Anim_TestAbsFrameRange(
        item->frame_num, Item_GetAnim(item)->frame_base + start,
        Item_GetAnim(item)->frame_base + end);
}

bool Item_GetAnimChange(ITEM *const item, const ANIM *const anim)
{
    if (item->current_anim_state == item->goal_anim_state) {
        return false;
    }

    for (int32_t i = 0; i < anim->num_changes; i++) {
        const ANIM_CHANGE *const change = Anim_GetChange(anim->change_idx + i);
        if (change->goal_anim_state != item->goal_anim_state) {
            continue;
        }

        for (int32_t j = 0; j < change->num_ranges; j++) {
            const ANIM_RANGE *const range =
                Anim_GetRange(change->range_idx + j);

            if (Anim_TestAbsFrameRange(
                    item->frame_num, range->start_frame, range->end_frame)) {
                item->anim_num = range->link_anim_num;
                item->frame_num = range->link_frame_num;
                return true;
            }
        }
    }

    return false;
}

void Item_Translate(
    ITEM *const item, const int32_t x, const int32_t y, const int32_t z)
{
    const int32_t c = Math_Cos(item->rot.y);
    const int32_t s = Math_Sin(item->rot.y);
    item->pos.x += ((c * x + s * z) >> W2V_SHIFT);
    item->pos.y += y;
    item->pos.z += ((c * z - s * x) >> W2V_SHIFT);
}

void Item_Animate(ITEM *const item)
{
    item->hit_status = 0;
    item->touch_bits = 0;
    item->frame_num++;

    const ANIM *anim = Item_GetAnim(item);
    if (anim->num_changes > 0 && Item_GetAnimChange(item, anim)) {
        anim = Item_GetAnim(item);
        item->current_anim_state = anim->current_anim_state;

        if (item->required_anim_state == anim->current_anim_state) {
            item->required_anim_state = 0;
        }
    }

    if (item->frame_num > anim->frame_end) {
        for (int32_t i = 0; i < anim->num_commands; i++) {
            const ANIM_COMMAND *const command = &anim->commands[i];
            switch (command->type) {
            case AC_MOVE_ORIGIN: {
                const XYZ_16 *const pos = (XYZ_16 *)command->data;
                Item_Translate(item, pos->x, pos->y, pos->z);
                break;
            }

            case AC_JUMP_VELOCITY: {
                const ANIM_COMMAND_VELOCITY_DATA *const data =
                    (ANIM_COMMAND_VELOCITY_DATA *)command->data;
                item->fall_speed = data->fall_speed;
                item->speed = data->speed;
                item->gravity = true;
                break;
            }

            case AC_DEACTIVATE:
                item->status = IS_DEACTIVATED;
                break;

            default:
                break;
            }
        }

        item->anim_num = anim->jump_anim_num;
        item->frame_num = anim->jump_frame_num;
        anim = Item_GetAnim(item);

#if TR_VERSION == 1
        item->current_anim_state = anim->current_anim_state;
        item->goal_anim_state = item->current_anim_state;
#else
        if (item->current_anim_state != anim->current_anim_state) {
            item->current_anim_state = anim->current_anim_state;
            item->goal_anim_state = anim->current_anim_state;
        }
#endif

        if (item->required_anim_state == item->current_anim_state) {
            item->required_anim_state = 0;
        }
    }

    for (int32_t i = 0; i < anim->num_commands; i++) {
        const ANIM_COMMAND *const command = &anim->commands[i];
        switch (command->type) {
        case AC_SOUND_FX: {
            const ANIM_COMMAND_EFFECT_DATA *const data =
                (ANIM_COMMAND_EFFECT_DATA *)command->data;
            Item_PlayAnimSFX(item, data);
            break;
        }

        case AC_EFFECT:
            const ANIM_COMMAND_EFFECT_DATA *const data =
                (ANIM_COMMAND_EFFECT_DATA *)command->data;
            if (item->frame_num == data->frame_num) {
                ItemAction_Run(data->effect_num, item);
            }
            break;

        default:
            break;
        }
    }

    if (item->gravity != 0) {
        item->fall_speed += item->fall_speed < FAST_FALL_SPEED ? GRAVITY : 1;
        item->pos.y += item->fall_speed;
    } else {
        int32_t speed = anim->velocity;
        if (anim->acceleration != 0) {
            speed += anim->acceleration * (item->frame_num - anim->frame_base);
        }
        item->speed = speed >> 16;
    }

    item->pos.x += (item->speed * Math_Sin(item->rot.y)) >> W2V_SHIFT;
    item->pos.z += (item->speed * Math_Cos(item->rot.y)) >> W2V_SHIFT;
}

void Item_PlayAnimSFX(
    const ITEM *const item, const ANIM_COMMAND_EFFECT_DATA *const data)
{
    if (item->frame_num != data->frame_num) {
        return;
    }

    const ITEM *const lara_item = Lara_GetItem();
    const ANIM_COMMAND_ENVIRONMENT mode = data->environment;

    if (mode != ACE_ALL && item->room_num != NO_ROOM) {
        int32_t height = NO_HEIGHT;
        if (item == lara_item) {
            height = Lara_GetLaraInfo()->water_surface_dist;
        } else if (Room_Get(item->room_num)->flags & RF_UNDERWATER) {
            height = -STEP_L;
        }

        if ((mode == ACE_WATER && (height >= 0 || height == NO_HEIGHT))
            || (mode == ACE_LAND && height < 0 && height != NO_HEIGHT)) {
            return;
        }
    }

    SOUND_PLAY_MODE play_mode = SPM_NORMAL;
    if (item == lara_item) {
        play_mode = SPM_ALWAYS;
    } else if (Object_IsObjectType(item->object_id, g_WaterObjects)) {
        play_mode = SPM_UNDERWATER;
    }
#if TR_VERSION > 1
    else if (item->object_id == O_LARA_HARPOON) {
        play_mode = SPM_ALWAYS;
    }
#endif

    Sound_Effect(data->effect_num, &item->pos, play_mode);
}
