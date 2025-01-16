#include "game/items.h"

#include "game/carrier.h"
#include "game/effects.h"
#include "game/interpolation.h"
#include "game/item_actions.h"
#include "game/random.h"
#include "game/room.h"
#include "game/shell.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/math.h>
#include <libtrx/game/matrix.h>
#include <libtrx/utils.h>

#include <stddef.h>

#define ITEM_ADJUST_ROT(source, target, rot)                                   \
    do {                                                                       \
        if ((int16_t)(target - source) > rot) {                                \
            source += rot;                                                     \
        } else if ((int16_t)(target - source) < -rot) {                        \
            source -= rot;                                                     \
        } else {                                                               \
            source = target;                                                   \
        }                                                                      \
    } while (0)

ITEM *g_Items = NULL;
int16_t g_NextItemActive = NO_ITEM;
static int16_t m_NextItemFree = NO_ITEM;
static BOUNDS_16 m_InterpolatedBounds = {};
static int16_t m_MaxUsedItemCount = 0;

void Item_InitialiseArray(int32_t num_items)
{
    g_NextItemActive = NO_ITEM;
    m_NextItemFree = g_LevelItemCount;
    m_MaxUsedItemCount = g_LevelItemCount;
    for (int i = g_LevelItemCount; i < num_items - 1; i++) {
        g_Items[i].next_item = i + 1;
    }
    g_Items[num_items - 1].next_item = NO_ITEM;
}

int32_t Item_GetTotalCount(void)
{
    return m_MaxUsedItemCount;
}

void Item_Control(void)
{
    int16_t item_num = g_NextItemActive;
    while (item_num != NO_ITEM) {
        ITEM *item = &g_Items[item_num];
        OBJECT *obj = &g_Objects[item->object_id];
        if (obj->control) {
            obj->control(item_num);
        }
        item_num = item->next_active;
    }

    Carrier_AnimateDrops();
}

void Item_Kill(int16_t item_num)
{
    ITEM *item = &g_Items[item_num];

    Item_RemoveActive(item_num);
    Item_RemoveDrawn(item_num);

    if (item == g_Lara.target) {
        g_Lara.target = NULL;
    }

    item->hit_points = -1;
    item->flags |= IF_KILLED;
    if (item_num >= g_LevelItemCount) {
        item->next_item = m_NextItemFree;
        m_NextItemFree = item_num;
    }

    while (m_MaxUsedItemCount > 0
           && g_Items[m_MaxUsedItemCount - 1].flags & IF_KILLED) {
        m_MaxUsedItemCount--;
    }
}

int16_t Item_Create(void)
{
    int16_t item_num = m_NextItemFree;
    if (item_num != NO_ITEM) {
        g_Items[item_num].flags = 0;
        m_NextItemFree = g_Items[item_num].next_item;
    }
    m_MaxUsedItemCount = MAX(m_MaxUsedItemCount, item_num + 1);
    return item_num;
}

void Item_Initialise(int16_t item_num)
{
    ITEM *item = &g_Items[item_num];
    OBJECT *object = &g_Objects[item->object_id];

    Item_SwitchToAnim(item, 0, 0);
    item->current_anim_state = Item_GetAnim(item)->current_anim_state;
    item->goal_anim_state = item->current_anim_state;
    item->required_anim_state = 0;
    item->rot.x = 0;
    item->rot.z = 0;
    item->speed = 0;
    item->fall_speed = 0;
    item->status = IS_INACTIVE;
    item->active = 0;
    item->gravity = 0;
    item->hit_status = 0;
    item->looked_at = 0;
    item->collidable = 1;
    item->hit_points = object->hit_points;
    item->timer = 0;
    item->mesh_bits = -1;
    item->touch_bits = 0;
    item->data = NULL;
    item->priv = NULL;
    item->carried_item = NULL;
    item->enable_shadow = true;

    if (item->flags & IF_INVISIBLE) {
        item->status = IS_INVISIBLE;
        item->flags &= ~IF_INVISIBLE;
    }

    if ((item->flags & IF_CODE_BITS) == IF_CODE_BITS) {
        item->flags &= ~IF_CODE_BITS;
        item->flags |= IF_REVERSE;
        Item_AddActive(item_num);
        item->status = IS_ACTIVE;
    }

    ROOM *const r = &g_RoomInfo[item->room_num];
    item->next_item = r->item_num;
    r->item_num = item_num;
    const int32_t z_sector = (item->pos.z - r->pos.z) >> WALL_SHIFT;
    const int32_t x_sector = (item->pos.x - r->pos.x) >> WALL_SHIFT;
    const SECTOR *const sector = &r->sectors[z_sector + x_sector * r->size.z];
    item->floor = sector->floor.height;

    if (g_GameInfo.bonus_flag & GBF_NGPLUS) {
        item->hit_points *= 2;
    }
    if (object->initialise) {
        object->initialise(item_num);
    }

    Interpolation_RememberItem(item);
}

void Item_RemoveActive(int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    if (!item->active) {
        return;
    }

    item->active = 0;

    int16_t link_num = g_NextItemActive;
    if (link_num == item_num) {
        g_NextItemActive = item->next_active;
        return;
    }

    while (link_num != NO_ITEM) {
        if (g_Items[link_num].next_active == item_num) {
            g_Items[link_num].next_active = item->next_active;
            return;
        }
        link_num = g_Items[link_num].next_active;
    }
}

void Item_RemoveDrawn(int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    ROOM *const r = &g_RoomInfo[item->room_num];

    int16_t link_num = r->item_num;
    if (link_num == item_num) {
        r->item_num = item->next_item;
        return;
    }

    while (link_num != NO_ITEM) {
        if (g_Items[link_num].next_item == item_num) {
            g_Items[link_num].next_item = item->next_item;
            return;
        }
        link_num = g_Items[link_num].next_item;
    }
}

void Item_AddActive(int16_t item_num)
{
    ITEM *item = &g_Items[item_num];

    if (!g_Objects[item->object_id].control) {
        item->status = IS_INACTIVE;
        return;
    }

    if (item->active) {
        return;
    }

    item->active = 1;
    item->next_active = g_NextItemActive;
    g_NextItemActive = item_num;
}

void Item_NewRoom(int16_t item_num, int16_t room_num)
{
    ITEM *item = &g_Items[item_num];
    ROOM *r = &g_RoomInfo[item->room_num];

    if (item->room_num != NO_ROOM) {
        int16_t linknum = r->item_num;
        if (linknum == item_num) {
            r->item_num = item->next_item;
        } else {
            for (; linknum != NO_ITEM; linknum = g_Items[linknum].next_item) {
                if (g_Items[linknum].next_item == item_num) {
                    g_Items[linknum].next_item = item->next_item;
                    break;
                }
            }
        }
    }

    r = &g_RoomInfo[room_num];
    item->room_num = room_num;
    item->next_item = r->item_num;
    r->item_num = item_num;
}

void Item_UpdateRoom(ITEM *item, int32_t height)
{
    int32_t x = item->pos.x;
    int32_t y = item->pos.y + height;
    int32_t z = item->pos.z;
    int16_t room_num = item->room_num;
    const SECTOR *const sector = Room_GetSector(x, y, z, &room_num);
    item->floor = Room_GetHeight(sector, x, y, z);
    if (item->room_num != room_num) {
        Item_NewRoom(g_Lara.item_num, room_num);
    }
}

int16_t Item_GetHeight(ITEM *item)
{
    int16_t room_num = item->room_num;
    const SECTOR *const sector =
        Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);
    const int32_t height =
        Room_GetHeight(sector, item->pos.x, item->pos.y, item->pos.z);

    return height;
}

int16_t Item_GetWaterHeight(ITEM *item)
{
    int16_t height = Room_GetWaterHeight(
        item->pos.x, item->pos.y, item->pos.z, item->room_num);
    if (height != NO_HEIGHT) {
        height -= item->pos.y;
    }

    return height;
}

int16_t Item_Spawn(const ITEM *const item, const GAME_OBJECT_ID object_id)
{
    int16_t spawn_num = Item_Create();
    if (spawn_num != NO_ITEM) {
        ITEM *spawn = &g_Items[spawn_num];
        spawn->object_id = object_id;
        spawn->room_num = item->room_num;
        spawn->pos = item->pos;
        spawn->rot = item->rot;
        Item_Initialise(spawn_num);
        spawn->status = IS_INACTIVE;
        spawn->shade = HIGH_LIGHT;
    }
    return spawn_num;
}

int32_t Item_GlobalReplace(
    const GAME_OBJECT_ID src_object_id, const GAME_OBJECT_ID dst_object_id)
{
    int32_t changed = 0;
    for (int i = 0; i < g_RoomCount; i++) {
        ROOM *r = &g_RoomInfo[i];
        for (int16_t item_num = r->item_num; item_num != NO_ITEM;
             item_num = g_Items[item_num].next_item) {
            if (g_Items[item_num].object_id == src_object_id) {
                g_Items[item_num].object_id = dst_object_id;
                changed++;
            }
        }
    }
    return changed;
}

bool Item_IsNearItem(const ITEM *item, const XYZ_32 *pos, int32_t distance)
{
    int32_t x = pos->x - item->pos.x;
    int32_t y = pos->y - item->pos.y;
    int32_t z = pos->z - item->pos.z;

    if (x >= -distance && x <= distance && z >= -distance && z <= distance
        && y >= -WALL_L * 3 && y <= WALL_L * 3
        && SQUARE(x) + SQUARE(z) <= SQUARE(distance)) {
        const BOUNDS_16 *const bounds = Item_GetBoundsAccurate(item);
        if (y >= bounds->min.y && y <= bounds->max.y + 100) {
            return true;
        }
    }

    return false;
}

int32_t Item_GetDistance(const ITEM *const item, const XYZ_32 *const target)
{
    int32_t x = (item->pos.x - target->x);
    int32_t y = (item->pos.y - target->y);
    int32_t z = (item->pos.z - target->z);

    int32_t scale = 0;
    while ((int16_t)x != x || (int16_t)y != y || (int16_t)z != z) {
        scale++;
        x >>= 1;
        y >>= 1;
        z >>= 1;
    }

    return Math_Sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z)) << scale;
}

bool Item_Test3DRange(int32_t x, int32_t y, int32_t z, int32_t range)
{
    return ABS(x) < range && ABS(y) < range && ABS(z) < range
        && (SQUARE(x) + SQUARE(y) + SQUARE(z) < SQUARE(range));
}

bool Item_TestBoundsCollide(ITEM *src_item, ITEM *dst_item, int32_t radius)
{
    const BOUNDS_16 *const src_bounds = &Item_GetBestFrame(src_item)->bounds;
    const BOUNDS_16 *const dst_bounds = &Item_GetBestFrame(dst_item)->bounds;
    if (dst_item->pos.y + dst_bounds->max.y
            <= src_item->pos.y + src_bounds->min.y
        || dst_item->pos.y + dst_bounds->min.y
            >= src_item->pos.y + src_bounds->max.y) {
        return false;
    }

    const int32_t c = Math_Cos(dst_item->rot.y);
    const int32_t s = Math_Sin(dst_item->rot.y);
    const int32_t x = src_item->pos.x - dst_item->pos.x;
    const int32_t z = src_item->pos.z - dst_item->pos.z;
    const int32_t rx = (c * x - s * z) >> W2V_SHIFT;
    const int32_t rz = (c * z + s * x) >> W2V_SHIFT;
    const int32_t min_x = dst_bounds->min.x - radius;
    const int32_t max_x = dst_bounds->max.x + radius;
    const int32_t min_z = dst_bounds->min.z - radius;
    const int32_t max_z = dst_bounds->max.z + radius;
    return rx >= min_x && rx <= max_x && rz >= min_z && rz <= max_z;
}

bool Item_TestPosition(
    const ITEM *const src_item, const ITEM *const dst_item,
    const OBJECT_BOUNDS *const bounds)
{
    const XYZ_16 rot = {
        .x = src_item->rot.x - dst_item->rot.x,
        .y = src_item->rot.y - dst_item->rot.y,
        .z = src_item->rot.z - dst_item->rot.z,
    };
    if (rot.x < bounds->rot.min.x || rot.x > bounds->rot.max.x
        || rot.y < bounds->rot.min.y || rot.y > bounds->rot.max.y
        || rot.z < bounds->rot.min.z || rot.z > bounds->rot.max.z) {
        return false;
    }

    const XYZ_32 dist = {
        .x = src_item->pos.x - dst_item->pos.x,
        .y = src_item->pos.y - dst_item->pos.y,
        .z = src_item->pos.z - dst_item->pos.z,
    };

    Matrix_PushUnit();
    Matrix_Rot16(dst_item->rot);
    MATRIX *mptr = g_MatrixPtr;
    const XYZ_32 shift = {
        .x = (mptr->_00 * dist.x + mptr->_10 * dist.y + mptr->_20 * dist.z)
            >> W2V_SHIFT,
        .y = (mptr->_01 * dist.x + mptr->_11 * dist.y + mptr->_21 * dist.z)
            >> W2V_SHIFT,
        .z = (mptr->_02 * dist.x + mptr->_12 * dist.y + mptr->_22 * dist.z)
            >> W2V_SHIFT,
    };
    Matrix_Pop();

    if (shift.x < bounds->shift.min.x || shift.x > bounds->shift.max.x
        || shift.y < bounds->shift.min.y || shift.y > bounds->shift.max.y
        || shift.z < bounds->shift.min.z || shift.z > bounds->shift.max.z) {
        return false;
    }

    return true;
}

void Item_AlignPosition(ITEM *src_item, ITEM *dst_item, XYZ_32 *vec)
{
    src_item->rot.x = dst_item->rot.x;
    src_item->rot.y = dst_item->rot.y;
    src_item->rot.z = dst_item->rot.z;

    Matrix_PushUnit();
    Matrix_Rot16(dst_item->rot);
    MATRIX *mptr = g_MatrixPtr;
    src_item->pos.x = dst_item->pos.x
        + ((mptr->_00 * vec->x + mptr->_01 * vec->y + mptr->_02 * vec->z)
           >> W2V_SHIFT);
    src_item->pos.y = dst_item->pos.y
        + ((mptr->_10 * vec->x + mptr->_11 * vec->y + mptr->_12 * vec->z)
           >> W2V_SHIFT);
    src_item->pos.z = dst_item->pos.z
        + ((mptr->_20 * vec->x + mptr->_21 * vec->y + mptr->_22 * vec->z)
           >> W2V_SHIFT);
    Matrix_Pop();
}

bool Item_MovePosition(
    ITEM *item, const ITEM *ref_item, const XYZ_32 *vec, int32_t velocity)
{
    const XYZ_32 *ref_pos = &ref_item->pos;

    Matrix_PushUnit();
    Matrix_Rot16(ref_item->rot);

    MATRIX *mptr = g_MatrixPtr;
    const XYZ_32 dst_pos = {
        .x = ref_pos->x
            + ((mptr->_00 * vec->x + mptr->_01 * vec->y + mptr->_02 * vec->z)
               >> W2V_SHIFT),
        .y = ref_pos->y
            + ((mptr->_10 * vec->x + mptr->_11 * vec->y + mptr->_12 * vec->z)
               >> W2V_SHIFT),
        .z = ref_pos->z
            + ((mptr->_20 * vec->x + mptr->_21 * vec->y + mptr->_22 * vec->z)
               >> W2V_SHIFT),
    };

    const XYZ_16 dst_rot = ref_item->rot;

    Matrix_Pop();

    {
        const int32_t dx = dst_pos.x - item->pos.x;
        const int32_t dy = dst_pos.y - item->pos.y;
        const int32_t dz = dst_pos.z - item->pos.z;
        const int32_t dist = Math_Sqrt(SQUARE(dx) + SQUARE(dy) + SQUARE(dz));
        if (velocity >= dist) {
            item->pos.x = dst_pos.x;
            item->pos.y = dst_pos.y;
            item->pos.z = dst_pos.z;
        } else {
            item->pos.x += (dx * velocity) / dist;
            item->pos.y += (dy * velocity) / dist;
            item->pos.z += (dz * velocity) / dist;
        }
    }

    if (item == g_LaraItem && g_Config.gameplay.enable_walk_to_items
        && !g_Lara.interact_target.is_moving) {
        if (g_Lara.water_status != LWS_UNDERWATER) {
            const int16_t step_to_anim_num[4] = {
                LA_SIDE_STEP_LEFT,
                LA_WALK_FORWARD,
                LA_SIDE_STEP_RIGHT,
                LA_WALK_BACK,
            };
            const int16_t step_to_anim_state[4] = {
                LS_STEP_LEFT,
                LS_WALK,
                LS_STEP_RIGHT,
                LS_BACK,
            };

            const int32_t dx = item->pos.x - dst_pos.x;
            const int32_t dz = item->pos.z - dst_pos.z;
            const int32_t angle = (DEG_360 - Math_Atan(dx, dz)) % DEG_360;
            const uint32_t src_quadrant = (uint32_t)(angle + DEG_45) / DEG_90;
            const uint32_t dst_quadrant =
                (uint32_t)(dst_rot.y + DEG_45) / DEG_90;
            const DIRECTION quadrant = (src_quadrant - dst_quadrant) % 4;

            Item_SwitchToAnim(item, step_to_anim_num[quadrant], 0);
            item->goal_anim_state = step_to_anim_state[quadrant];
            item->current_anim_state = step_to_anim_state[quadrant];

            g_Lara.gun_status = LGS_HANDS_BUSY;
        }

        g_Lara.interact_target.is_moving = true;
        g_Lara.interact_target.move_count = 0;
    }

    int16_t rotation = MOVE_ANG;
    ITEM_ADJUST_ROT(item->rot.x, dst_rot.x, rotation);
    ITEM_ADJUST_ROT(item->rot.y, dst_rot.y, rotation);
    ITEM_ADJUST_ROT(item->rot.z, dst_rot.z, rotation);

    // clang-format off
    return item->pos.x == dst_pos.x
        && item->pos.y == dst_pos.y
        && item->pos.z == dst_pos.z
        && item->rot.x == dst_rot.x
        && item->rot.y == dst_rot.y
        && item->rot.z == dst_rot.z;
    // clang-format on
}

void Item_ShiftCol(ITEM *item, COLL_INFO *coll)
{
    item->pos.x += coll->shift.x;
    item->pos.y += coll->shift.y;
    item->pos.z += coll->shift.z;
    coll->shift.x = 0;
    coll->shift.y = 0;
    coll->shift.z = 0;
}

void Item_Translate(ITEM *item, int32_t x, int32_t y, int32_t z)
{
    int32_t c = Math_Cos(item->rot.y);
    int32_t s = Math_Sin(item->rot.y);
    item->pos.x += (c * x + s * z) >> W2V_SHIFT;
    item->pos.y += y;
    item->pos.z += (c * z - s * x) >> W2V_SHIFT;
}

void Item_Animate(ITEM *item)
{
    item->touch_bits = 0;
    item->hit_status = 0;

    const ANIM *anim = Item_GetAnim(item);

    item->frame_num++;

    if (anim->num_changes > 0) {
        if (Item_GetAnimChange(item, anim)) {
            anim = Item_GetAnim(item);
            item->current_anim_state = anim->current_anim_state;

            if (item->required_anim_state == item->current_anim_state) {
                item->required_anim_state = 0;
            }
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
        item->current_anim_state = anim->current_anim_state;
        item->goal_anim_state = item->current_anim_state;
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

        case AC_EFFECT: {
            const ANIM_COMMAND_EFFECT_DATA *const data =
                (ANIM_COMMAND_EFFECT_DATA *)command->data;
            if (item->frame_num == data->frame_num) {
                ItemAction_Run(data->effect_num, item);
            }
            break;
        }

        default:
            break;
        }
    }

    if (!item->gravity) {
        int32_t speed = anim->velocity;
        if (anim->acceleration) {
            speed += anim->acceleration * (item->frame_num - anim->frame_base);
        }
        item->speed = speed >> 16;
    } else {
        item->fall_speed += (item->fall_speed < FAST_FALL_SPEED) ? GRAVITY : 1;
        item->pos.y += item->fall_speed;
    }

    item->pos.x += (Math_Sin(item->rot.y) * item->speed) >> W2V_SHIFT;
    item->pos.z += (Math_Cos(item->rot.y) * item->speed) >> W2V_SHIFT;
}

bool Item_IsTriggerActive(ITEM *item)
{
    bool ok = !(item->flags & IF_REVERSE);

    if ((item->flags & IF_CODE_BITS) != IF_CODE_BITS) {
        return !ok;
    }

    if (!item->timer) {
        return ok;
    }

    if (item->timer == -1) {
        return !ok;
    }

    item->timer--;

    if (!item->timer) {
        item->timer = -1;
    }

    return ok;
}

ANIM_FRAME *Item_GetBestFrame(const ITEM *item)
{
    ANIM_FRAME *frmptr[2];
    int32_t rate;
    int32_t frac = Item_GetFrames(item, frmptr, &rate);
    if (frac <= rate / 2) {
        return frmptr[0];
    } else {
        return frmptr[1];
    }
}

const BOUNDS_16 *Item_GetBoundsAccurate(const ITEM *item)
{
    int32_t rate;
    ANIM_FRAME *frmptr[2];

    int32_t frac = Item_GetFrames(item, frmptr, &rate);
    if (!frac) {
        return &frmptr[0]->bounds;
    }

    const BOUNDS_16 *const a = &frmptr[0]->bounds;
    const BOUNDS_16 *const b = &frmptr[1]->bounds;
    BOUNDS_16 *const result = &m_InterpolatedBounds;

    result->min.x = a->min.x + (((b->min.x - a->min.x) * frac) / rate);
    result->min.y = a->min.y + (((b->min.y - a->min.y) * frac) / rate);
    result->min.z = a->min.z + (((b->min.z - a->min.z) * frac) / rate);
    result->max.x = a->max.x + (((b->max.x - a->max.x) * frac) / rate);
    result->max.y = a->max.y + (((b->max.y - a->max.y) * frac) / rate);
    result->max.z = a->max.z + (((b->max.z - a->max.z) * frac) / rate);
    return result;
}

int32_t Item_GetFrames(const ITEM *item, ANIM_FRAME *frmptr[], int32_t *rate)
{
    const ANIM *const anim = Item_GetAnim(item);

    const int32_t cur_frame_num = item->frame_num - anim->frame_base;
    const int32_t last_frame_num = anim->frame_end - anim->frame_base;
    const int32_t key_frame_span = anim->interpolation;
    const int32_t first_key_frame_num = cur_frame_num / key_frame_span;
    const int32_t second_key_frame_num = first_key_frame_num + 1;

    frmptr[0] = &anim->frame_ptr[first_key_frame_num];
    frmptr[1] = &anim->frame_ptr[second_key_frame_num];

    const int32_t key_frame_shift = cur_frame_num % key_frame_span;
    const int32_t numerator = key_frame_shift;
    int32_t denominator = key_frame_span;
    if (numerator && second_key_frame_num > anim->frame_end) {
        denominator = anim->frame_end + key_frame_span - second_key_frame_num;
    }

    // OG
    if (g_Config.rendering.fps == 30) {
        *rate = denominator;
        return numerator;
    }

    // interpolated
    if ((item != g_LaraItem && !item->active) || (item->object_id == O_STATUE)
        || (item->object_id == O_ROLLING_BALL && item->status != IS_ACTIVE)) {
        *rate = denominator;
        return numerator;
    }

    const double clock_ratio = Interpolation_GetRate() - 0.5;
    const double final =
        (key_frame_shift + clock_ratio) / (double)key_frame_span;
    const double interp_frame_num =
        (first_key_frame_num * key_frame_span) + (final * key_frame_span);
    if (interp_frame_num >= last_frame_num) {
        *rate = denominator;
        return numerator;
    }

    *rate = 10;
    return final * 10;
}

ITEM *Item_Get(const int16_t item_num)
{
    if (item_num == NO_ITEM) {
        return NULL;
    }
    return &g_Items[item_num];
}

int32_t Item_Explode(int16_t item_num, int32_t mesh_bits, int16_t damage)
{
    ITEM *const item = Item_Get(item_num);
    const OBJECT *const obj = Object_GetObject(item->object_id);

    const ANIM_FRAME *const frame = Item_GetBestFrame(item);

    Matrix_PushUnit();
    Matrix_Rot16(item->rot);
    Matrix_TranslateRel16(frame->offset);
    Matrix_Rot16(frame->mesh_rots[0]);

#if 0
    // XXX: present in OG, removed by GLrage on the grounds that it sometimes
    // crashes.
    int16_t *extra_rotation = (int16_t*)item->data;
#endif

    int32_t bit = 1;
    if ((bit & mesh_bits) && (bit & item->mesh_bits)) {
        int16_t effect_num = Effect_Create(item->room_num);
        if (effect_num != NO_EFFECT) {
            EFFECT *effect = Effect_Get(effect_num);
            effect->room_num = item->room_num;
            effect->pos.x = (g_MatrixPtr->_03 >> W2V_SHIFT) + item->pos.x;
            effect->pos.y = (g_MatrixPtr->_13 >> W2V_SHIFT) + item->pos.y;
            effect->pos.z = (g_MatrixPtr->_23 >> W2V_SHIFT) + item->pos.z;
            effect->rot.y = (Random_GetControl() - 0x4000) * 2;
            if (item->object_id == O_TORSO) {
                effect->speed = Random_GetControl() >> 7;
                effect->fall_speed = -Random_GetControl() >> 7;
            } else {
                effect->speed = Random_GetControl() >> 8;
                effect->fall_speed = -Random_GetControl() >> 8;
            }
            effect->counter = damage;
            effect->frame_num = obj->mesh_idx;
            effect->object_id = O_BODY_PART;
        }
        item->mesh_bits -= bit;
    }

    for (int i = 1; i < obj->mesh_count; i++) {
        const ANIM_BONE *const bone = Object_GetBone(obj, i - 1);
        if (bone->matrix_pop) {
            Matrix_Pop();
        }
        if (bone->matrix_push) {
            Matrix_Push();
        }

        Matrix_TranslateRel32(bone->pos);
        Matrix_Rot16(frame->mesh_rots[i]);

#if 0
    if (extra_rotation) {
        if (bone->rot_y) {
            Matrix_RotY(*extra_rotation++);
        }
        if (bone->rot_x) {
            Matrix_RotX(*extra_rotation++);
        }
        if (bone->rot_z) {
            Matrix_RotZ(*extra_rotation++);
        }
    }
#endif

        bit <<= 1;
        if ((bit & mesh_bits) && (bit & item->mesh_bits)) {
            int16_t effect_num = Effect_Create(item->room_num);
            if (effect_num != NO_EFFECT) {
                EFFECT *effect = Effect_Get(effect_num);
                effect->room_num = item->room_num;
                effect->pos.x = (g_MatrixPtr->_03 >> W2V_SHIFT) + item->pos.x;
                effect->pos.y = (g_MatrixPtr->_13 >> W2V_SHIFT) + item->pos.y;
                effect->pos.z = (g_MatrixPtr->_23 >> W2V_SHIFT) + item->pos.z;
                effect->rot.y = (Random_GetControl() - 0x4000) * 2;
                if (item->object_id == O_TORSO) {
                    effect->speed = Random_GetControl() >> 7;
                    effect->fall_speed = -Random_GetControl() >> 7;
                } else {
                    effect->speed = Random_GetControl() >> 8;
                    effect->fall_speed = -Random_GetControl() >> 8;
                }
                effect->counter = damage;
                effect->object_id = O_BODY_PART;
                effect->frame_num = obj->mesh_idx + i;
            }
            item->mesh_bits -= bit;
        }
    }

    Matrix_Pop();

    return !(item->mesh_bits & (0x7FFFFFFF >> (31 - obj->mesh_count)));
}
