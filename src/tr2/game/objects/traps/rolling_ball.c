#include "game/objects/traps/rolling_ball.h"

#include "game/gamebuf.h"
#include "game/items.h"
#include "game/math.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

#define ROLLING_BALL_DAMAGE_AIR 100
#define ROLL_SHAKE_RANGE (WALL_L * 10) // = 10240

void __cdecl RollingBall_Initialise(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    GAME_VECTOR *const data =
        GameBuf_Alloc(sizeof(GAME_VECTOR), GBUF_ROLLING_BALL_STUFF);
    data->pos.x = item->pos.x;
    data->pos.y = item->pos.y;
    data->pos.z = item->pos.z;
    data->room_num = item->room_num;

    item->data = data;
}

void __cdecl RollingBall_Control(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    if (item->status == IS_ACTIVE) {
        if (item->goal_anim_state == TRAP_WORKING) {
            Item_Animate(item);
            return;
        }

        if (item->pos.y < item->floor) {
            if (!item->gravity) {
                item->gravity = 1;
                item->fall_speed = -10;
            }
        } else if (item->current_anim_state == TRAP_SET) {
            item->goal_anim_state = TRAP_ACTIVATE;
        }

        const XYZ_32 old_pos = item->pos;
        Item_Animate(item);

        int16_t room_num = item->room_num;
        const SECTOR *const sector =
            Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);
        if (item->room_num != room_num) {
            Item_NewRoom(item_num, room_num);
        }

        item->floor =
            Room_GetHeight(sector, item->pos.x, item->pos.y, item->pos.z);

        Room_TestTriggers(g_TriggerIndex, true);

        if (item->pos.y >= item->floor - STEP_L) {
            item->gravity = 0;
            item->fall_speed = 0;
            item->pos.y = item->floor;
            if (item->object_id == O_ROLLING_BALL_2) {
                Sound_Effect(SFX_SNOWBALL_ROLL, &item->pos, SPM_NORMAL);
            } else if (item->object_id == O_ROLLING_BALL_3) {
                Sound_Effect(SFX_ROLLING_2, &item->pos, SPM_NORMAL);
            } else {
                Sound_Effect(SFX_ROLLING_BALL, &item->pos, SPM_NORMAL);
            }
            const int32_t dist = Math_Sqrt(
                (g_Camera.mic_pos.z - item->pos.z)
                    * (g_Camera.mic_pos.z - item->pos.z)
                + (g_Camera.mic_pos.x - item->pos.x)
                    * (g_Camera.mic_pos.x - item->pos.x));
            if (dist < ROLL_SHAKE_RANGE) {
                g_Camera.bounce =
                    40 * (dist - ROLL_SHAKE_RANGE) / ROLL_SHAKE_RANGE;
            }
        }

        {
            const int32_t dist =
                item->object_id == O_ROLLING_BALL_1 ? STEP_L * 3 / 2 : WALL_L;
            const int32_t x =
                item->pos.x + ((dist * Math_Sin(item->rot.y)) >> W2V_SHIFT);
            const int32_t z =
                item->pos.z + ((dist * Math_Cos(item->rot.y)) >> W2V_SHIFT);
            const SECTOR *const sector =
                Room_GetSector(x, item->pos.y, z, &room_num);
            if (Room_GetHeight(sector, x, item->pos.y, z) < item->pos.y) {
                if (item->object_id == O_ROLLING_BALL_2) {
                    Sound_Effect(SFX_SNOWBALL_STOP, &item->pos, SPM_NORMAL);
                    item->goal_anim_state = TRAP_WORKING;
                } else if (item->object_id == O_ROLLING_BALL_3) {
                    Sound_Effect(SFX_ROLLING_2_HIT, &item->pos, SPM_NORMAL);
                    item->goal_anim_state = TRAP_WORKING;
                } else {
                    item->status = IS_DEACTIVATED;
                }
                item->pos.x = old_pos.x;
                item->pos.y = item->floor;
                item->pos.z = old_pos.z;
                item->speed = 0;
                item->fall_speed = 0;
                item->touch_bits = 0;
            }
        }
    } else if (
        item->status == IS_DEACTIVATED
        && !Item_IsTriggerActive(&g_Items[item_num])) {
        item->status = IS_INACTIVE;
        const GAME_VECTOR *const data = item->data;
        item->pos.x = data->x;
        item->pos.y = data->y;
        item->pos.z = data->z;
        if (item->room_num != data->room_num) {
            Item_RemoveDrawn(item_num);
            ROOM *const r = Room_Get(data->room_num);
            item->next_item = r->item_num;
            r->item_num = item_num;
            item->room_num = data->room_num;
        }
        item->goal_anim_state = TRAP_SET;
        item->current_anim_state = TRAP_SET;
        item->anim_num = g_Objects[item->object_id].anim_idx;
        item->frame_num = g_Anims[item->anim_num].frame_base;
        item->goal_anim_state = g_Anims[item->anim_num].current_anim_state;
        item->current_anim_state = item->goal_anim_state;
        item->required_anim_state = TRAP_SET;
        Item_RemoveActive(item_num);
    }
}
