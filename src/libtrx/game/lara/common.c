#include "game/lara/common.h"

#include "game/const.h"
#include "game/item_actions.h"
#include "game/rooms/const.h"

void Lara_Animate(ITEM *const item)
{
    LARA_INFO *const lara = Lara_GetLaraInfo();
    item->frame_num++;

    const ANIM *anim = Item_GetAnim(item);
    if (anim->num_changes > 0 && Item_GetAnimChange(item, anim)) {
        anim = Item_GetAnim(item);
        item->current_anim_state = anim->current_anim_state;
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
                if (lara->calc_fall_speed != 0) {
                    item->fall_speed = lara->calc_fall_speed;
                    lara->calc_fall_speed = 0;
                }
                break;
            }

            case AC_ATTACK_READY:
                if (lara->gun_status != LGS_SPECIAL) {
                    lara->gun_status = LGS_ARMLESS;
                }
                break;
            default:
                break;
            }
        }

        item->anim_num = anim->jump_anim_num;
        item->frame_num = anim->jump_frame_num;
        anim = Item_GetAnim(item);
        item->current_anim_state = anim->current_anim_state;
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

        case AC_EFFECT: {
            const ANIM_COMMAND_EFFECT_DATA *const data =
                (ANIM_COMMAND_EFFECT_DATA *)command->data;
            if (item->frame_num != data->frame_num) {
                break;
            }

            const ANIM_COMMAND_ENVIRONMENT type = data->environment;
            const int32_t height = lara->water_surface_dist;
            if ((type == ACE_WATER && (height >= 0 || height == NO_HEIGHT))
                || (type == ACE_LAND && height < 0 && height != NO_HEIGHT)) {
                break;
            }

            ItemAction_Run(data->effect_num, item);
            break;
        }

        default:
            break;
        }
    }

    if (item->gravity != 0) {
        int32_t speed = anim->velocity
            + anim->acceleration * (item->frame_num - anim->frame_base - 1);
        item->speed -= (int16_t)(speed >> 16);
        speed += anim->acceleration;
        item->speed += (int16_t)(speed >> 16);

        item->fall_speed += item->fall_speed < FAST_FALL_SPEED ? GRAVITY : 1;
        item->pos.y += item->fall_speed;
    } else {
        int32_t speed = anim->velocity;
        if (anim->acceleration != 0) {
            speed += anim->acceleration * (item->frame_num - anim->frame_base);
        }
        item->speed = (int16_t)(speed >> 16);
    }

    item->pos.x += (item->speed * Math_Sin(lara->move_angle)) >> W2V_SHIFT;
    item->pos.z += (item->speed * Math_Cos(lara->move_angle)) >> W2V_SHIFT;
}

void Lara_SwapSingleMesh(const LARA_MESH mesh, const GAME_OBJECT_ID obj_id)
{
    const OBJECT *const obj = Object_Get(obj_id);
    Lara_SetMesh(mesh, Object_GetMesh(obj->mesh_idx + mesh));
}

OBJECT_MESH *Lara_GetMesh(const LARA_MESH mesh)
{
    return Lara_GetLaraInfo()->mesh_ptrs[mesh];
}

void Lara_SetMesh(const LARA_MESH mesh, OBJECT_MESH *const mesh_ptr)
{
    Lara_GetLaraInfo()->mesh_ptrs[mesh] = mesh_ptr;
}

const ANIM_FRAME *Lara_GetHitFrame(const ITEM *const item)
{
    const LARA_INFO *const lara = Lara_GetLaraInfo();
    if (lara->hit_direction < 0) {
        return nullptr;
    }

    // clang-format off
    LARA_ANIMATION anim_idx;
    switch (lara->hit_direction) {
    case DIR_EAST:  anim_idx = LA_HIT_LEFT; break;
    case DIR_SOUTH: anim_idx = LA_HIT_BACK; break;
    case DIR_WEST:  anim_idx = LA_HIT_RIGHT; break;
    default:        anim_idx = LA_HIT_FRONT; break;
    }
    // clang-format on

    const OBJECT *const obj = Object_Get(item->object_id);
    const ANIM *const anim = Object_GetAnim(obj, anim_idx);
    return &anim->frame_ptr[lara->hit_frame];
}
