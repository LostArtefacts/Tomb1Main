#include "game/items.h"

#include "game/effects.h"
#include "game/gameflow/gameflow_new.h"
#include "game/item_actions.h"
#include "game/output.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/game/math.h>
#include <libtrx/game/matrix.h>
#include <libtrx/utils.h>

static int16_t m_NextItemFree;
static int16_t m_MaxUsedItemCount = 0;
static BOUNDS_16 m_InterpolatedBounds = {};

static OBJECT_BOUNDS M_ConvertBounds(const int16_t *bounds_in);

static OBJECT_BOUNDS M_ConvertBounds(const int16_t *const bounds_in)
{
    // TODO: remove this conversion utility once we gain control over its
    // incoming arguments
    return (OBJECT_BOUNDS) {
        .shift = {
            .min = {
                .x = bounds_in[0],
                .y = bounds_in[2],
                .z = bounds_in[4],
            },
            .max = {
                .x = bounds_in[1],
                .y = bounds_in[3],
                .z = bounds_in[5],
            },
        },
        .rot = {
            .min = {
                .x = bounds_in[6],
                .y = bounds_in[8],
                .z = bounds_in[10],
            },
            .max = {
                .x = bounds_in[7],
                .y = bounds_in[9],
                .z = bounds_in[11],
            },
        },
    };
}

void Item_InitialiseArray(const int32_t num_items)
{
    ASSERT(num_items > 0);
    m_NextItemFree = g_LevelItemCount;
    g_PrevItemActive = NO_ITEM;
    g_NextItemActive = NO_ITEM;
    m_MaxUsedItemCount = g_LevelItemCount;
    for (int32_t i = m_NextItemFree; i < num_items - 1; i++) {
        ITEM *const item = &g_Items[i];
        item->active = 0;
        item->next_item = i + 1;
    }
    g_Items[num_items - 1].next_item = NO_ITEM;
}

void Item_Control(void)
{
    int16_t item_num = g_NextItemActive;
    while (item_num != NO_ITEM) {
        const ITEM *const item = Item_Get(item_num);
        const int16_t next = item->next_active;
        const OBJECT *object = Object_GetObject(item->object_id);
        if (!(item->flags & IF_KILLED) && object->control != NULL) {
            object->control(item_num);
        }
        item_num = next;
    }
}

int32_t Item_GetTotalCount(void)
{
    return m_MaxUsedItemCount;
}

int16_t Item_Create(void)
{
    const int16_t item_num = m_NextItemFree;
    if (item_num != NO_ITEM) {
        g_Items[item_num].flags = 0;
        m_NextItemFree = g_Items[item_num].next_item;
    }
    m_MaxUsedItemCount = MAX(m_MaxUsedItemCount, item_num + 1);
    return item_num;
}

void Item_Kill(const int16_t item_num)
{
    Item_RemoveActive(item_num);
    Item_RemoveDrawn(item_num);

    ITEM *const item = &g_Items[item_num];
    if (item == g_Lara.target) {
        g_Lara.target = NULL;
    }

    if (item_num < g_LevelItemCount) {
        item->flags |= IF_KILLED;
    } else {
        item->next_item = m_NextItemFree;
        m_NextItemFree = item_num;
    }

    while (m_MaxUsedItemCount > 0
           && g_Items[m_MaxUsedItemCount - 1].flags & IF_KILLED) {
        m_MaxUsedItemCount--;
    }
}

void Item_Initialise(const int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    Item_SwitchToAnim(item, 0, 0);
    item->goal_anim_state = Item_GetAnim(item)->current_anim_state;
    item->current_anim_state = item->goal_anim_state;
    item->required_anim_state = 0;
    item->rot.x = 0;
    item->rot.z = 0;
    item->speed = 0;
    item->fall_speed = 0;
    item->hit_points = g_Objects[item->object_id].hit_points;
    item->timer = 0;
    item->mesh_bits = 0xFFFFFFFF;
    item->touch_bits = 0;
    item->data = NULL;

    item->active = 0;
    item->status = IS_INACTIVE;
    item->gravity = 0;
    item->hit_status = 0;
    item->collidable = 1;
    item->looked_at = 0;
    item->killed = 0;

    if ((item->flags & IF_INVISIBLE) != 0) {
        item->status = IS_INVISIBLE;
        item->flags &= ~IF_INVISIBLE;
    } else if (g_Objects[item->object_id].intelligent) {
        item->status = IS_INVISIBLE;
    }

    if (item->flags & IF_KILLED) {
        item->killed = 1;
        item->flags &= ~IF_KILLED;
    }

    if ((item->flags & IF_CODE_BITS) == IF_CODE_BITS) {
        item->flags &= ~IF_CODE_BITS;
        item->flags |= IF_REVERSE;
        Item_AddActive(item_num);
        item->status = IS_ACTIVE;
    }

    ROOM *const room = &g_Rooms[item->room_num];
    item->next_item = room->item_num;
    room->item_num = item_num;

    const int32_t dx = (item->pos.x - room->pos.x) >> WALL_SHIFT;
    const int32_t dz = (item->pos.z - room->pos.z) >> WALL_SHIFT;
    const SECTOR *const sector = &room->sectors[dx * room->size.z + dz];
    item->floor = sector->floor.height;

    if (g_SaveGame.bonus_flag && g_GameInfo.current_level.type != GFL_DEMO) {
        item->hit_points *= 2;
    }

    if (g_Objects[item->object_id].initialise != NULL) {
        g_Objects[item->object_id].initialise(item_num);
    }
}

void Item_RemoveActive(const int16_t item_num)
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

void Item_RemoveDrawn(const int16_t item_num)
{
    const ITEM *const item = &g_Items[item_num];
    if (item->room_num == NO_ROOM) {
        return;
    }

    int16_t link_num = g_Rooms[item->room_num].item_num;
    if (link_num == item_num) {
        g_Rooms[item->room_num].item_num = item->next_item;
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

void Item_AddActive(const int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    if (g_Objects[item->object_id].control == NULL) {
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

void Item_NewRoom(const int16_t item_num, const int16_t room_num)
{
    ITEM *const item = &g_Items[item_num];
    ROOM *room = NULL;

    if (item->room_num != NO_ROOM) {
        room = &g_Rooms[item->room_num];

        int16_t link_num = room->item_num;
        if (link_num == item_num) {
            room->item_num = item->next_item;
        } else {
            while (link_num != NO_ITEM) {
                if (g_Items[link_num].next_item == item_num) {
                    g_Items[link_num].next_item = item->next_item;
                    break;
                }
                link_num = g_Items[link_num].next_item;
            }
        }
    }

    item->room_num = room_num;
    room = &g_Rooms[room_num];
    item->next_item = room->item_num;
    room->item_num = item_num;
}

int32_t Item_GlobalReplace(
    const GAME_OBJECT_ID src_object_id, const GAME_OBJECT_ID dst_object_id)
{
    int32_t changed = 0;

    for (int32_t i = 0; i < g_RoomCount; i++) {
        int16_t j = g_Rooms[i].item_num;
        while (j != NO_ITEM) {
            ITEM *const item = &g_Items[j];
            if (item->object_id == src_object_id) {
                item->object_id = dst_object_id;
                changed++;
            }
            j = item->next_item;
        }
    }

    return changed;
}

void Item_ClearKilled(void)
{
    // Remove corpses and other killed items. Part of OG performance
    // improvements, generously used in Opera House and Barkhang Monastery
    int16_t link_num = g_PrevItemActive;
    while (link_num != NO_ITEM) {
        ITEM *const item = &g_Items[link_num];
        Item_Kill(link_num);
        link_num = item->next_active;
        item->next_active = NO_ITEM;
    }
    g_PrevItemActive = NO_ITEM;
}

bool Item_IsSmashable(const ITEM *item)
{
    return (item->object_id == O_WINDOW_1 || item->object_id == O_BELL);
}

void Item_ShiftCol(ITEM *const item, COLL_INFO *const coll)
{
    item->pos.x += coll->shift.x;
    item->pos.y += coll->shift.y;
    item->pos.z += coll->shift.z;
    coll->shift.z = 0;
    coll->shift.y = 0;
    coll->shift.x = 0;
}

void Item_UpdateRoom(ITEM *const item, const int32_t height)
{
    int32_t x = item->pos.x;
    int32_t y = height + item->pos.y;
    int32_t z = item->pos.z;

    int16_t room_num = item->room_num;
    const SECTOR *const sector = Room_GetSector(x, y, z, &room_num);
    item->floor = Room_GetHeight(sector, x, y, z);
    if (item->room_num != room_num) {
        Item_NewRoom(g_Lara.item_num, room_num);
    }
}

int32_t Item_TestBoundsCollide(
    const ITEM *const src_item, const ITEM *const dst_item,
    const int32_t radius)
{
    const BOUNDS_16 *const src_bounds = &Item_GetBestFrame(src_item)->bounds;
    const BOUNDS_16 *const dst_bounds = &Item_GetBestFrame(dst_item)->bounds;

    if (src_item->pos.y + src_bounds->max.y
            <= dst_item->pos.y + dst_bounds->min.y
        || src_item->pos.y + src_bounds->min.y
            >= dst_item->pos.y + dst_bounds->max.y) {
        return false;
    }

    const int32_t c = Math_Cos(src_item->rot.y);
    const int32_t s = Math_Sin(src_item->rot.y);
    const int32_t dx = dst_item->pos.x - src_item->pos.x;
    const int32_t dz = dst_item->pos.z - src_item->pos.z;
    const int32_t rx = (c * dx - s * dz) >> W2V_SHIFT;
    const int32_t rz = (c * dz + s * dx) >> W2V_SHIFT;

    // clang-format off
    return (
        rx >= src_bounds->min.x - radius &&
        rx <= src_bounds->max.x + radius &&
        rz >= src_bounds->min.z - radius &&
        rz <= src_bounds->max.z + radius);
    // clang-format on
}

int32_t Item_TestPosition(
    const int16_t *const bounds_in, const ITEM *const src_item,
    const ITEM *const dst_item)
{
    const OBJECT_BOUNDS bounds = M_ConvertBounds(bounds_in);

    const XYZ_16 rot = {
        .x = dst_item->rot.x - src_item->rot.x,
        .y = dst_item->rot.y - src_item->rot.y,
        .z = dst_item->rot.z - src_item->rot.z,
    };
    const XYZ_32 dist = {
        .x = dst_item->pos.x - src_item->pos.x,
        .y = dst_item->pos.y - src_item->pos.y,
        .z = dst_item->pos.z - src_item->pos.z,
    };

    // clang-format off
    if (rot.x < bounds.rot.min.x ||
        rot.x > bounds.rot.max.x ||
        rot.y < bounds.rot.min.y ||
        rot.y > bounds.rot.max.y ||
        rot.z < bounds.rot.min.z ||
        rot.z > bounds.rot.max.z
    ) {
        return false;
    }
    // clang-format on

    Matrix_PushUnit();
    Matrix_RotYXZ(src_item->rot.y, src_item->rot.x, src_item->rot.z);
    const MATRIX *const m = g_MatrixPtr;
    const XYZ_32 shift = {
        .x = (dist.x * m->_00 + dist.y * m->_10 + dist.z * m->_20) >> W2V_SHIFT,
        .y = (dist.x * m->_01 + dist.y * m->_11 + dist.z * m->_21) >> W2V_SHIFT,
        .z = (dist.x * m->_02 + dist.y * m->_12 + dist.z * m->_22) >> W2V_SHIFT,
    };
    Matrix_Pop();

    // clang-format off
    return (
        shift.x >= bounds.shift.min.x &&
        shift.x <= bounds.shift.max.x &&
        shift.y >= bounds.shift.min.y &&
        shift.y <= bounds.shift.max.y &&
        shift.z >= bounds.shift.min.z &&
        shift.z <= bounds.shift.max.z
    );
    // clang-format on
}

void Item_AlignPosition(
    const XYZ_32 *const vec, const ITEM *const src_item, ITEM *const dst_item)
{
    dst_item->rot = src_item->rot;
    Matrix_PushUnit();
    Matrix_RotYXZ(src_item->rot.y, src_item->rot.x, src_item->rot.z);
    const MATRIX *const m = g_MatrixPtr;
    const XYZ_32 shift = {
        .x = (vec->x * m->_00 + vec->y * m->_01 + vec->z * m->_02) >> W2V_SHIFT,
        .y = (vec->x * m->_10 + vec->y * m->_11 + vec->z * m->_12) >> W2V_SHIFT,
        .z = (vec->x * m->_20 + vec->y * m->_21 + vec->z * m->_22) >> W2V_SHIFT,
    };
    Matrix_Pop();

    const XYZ_32 new_pos = {
        .x = src_item->pos.x + shift.x,
        .y = src_item->pos.y + shift.y,
        .z = src_item->pos.z + shift.z,
    };

    int16_t room_num = dst_item->room_num;
    const SECTOR *const sector =
        Room_GetSector(new_pos.x, new_pos.y, new_pos.z, &room_num);
    const int32_t height =
        Room_GetHeight(sector, new_pos.x, new_pos.y, new_pos.z);
    const int32_t ceiling =
        Room_GetCeiling(sector, new_pos.x, new_pos.y, new_pos.z);

    if (ABS(height - dst_item->pos.y) > STEP_L
        || ABS(ceiling - dst_item->pos.y) < LARA_HEIGHT) {
        return;
    }

    dst_item->pos.x = new_pos.x;
    dst_item->pos.y = new_pos.y;
    dst_item->pos.z = new_pos.z;
}

void Item_Animate(ITEM *const item)
{
    item->hit_status = 0;
    item->touch_bits = 0;

    const ANIM *anim = Item_GetAnim(item);

    item->frame_num++;

    if (anim->num_changes > 0) {
        if (Item_GetAnimChange(item, anim)) {
            anim = Item_GetAnim(item);
            item->current_anim_state = anim->current_anim_state;

            if (item->required_anim_state == anim->current_anim_state) {
                item->required_anim_state = 0;
            }
        }
    }

    if (item->frame_num > anim->frame_end) {
        if (anim->num_commands > 0) {
            const int16_t *cmd_ptr = Anim_GetCommand(anim->command_idx);

            for (int32_t i = 0; i < anim->num_commands; i++) {
                const int16_t cmd = *cmd_ptr++;

                switch (cmd) {
                case AC_MOVE_ORIGIN:
                    Item_Translate(item, cmd_ptr[0], cmd_ptr[1], cmd_ptr[2]);
                    cmd_ptr += 3;
                    break;

                case AC_JUMP_VELOCITY:
                    item->fall_speed = cmd_ptr[0];
                    item->speed = cmd_ptr[1];
                    item->gravity = 1;
                    cmd_ptr += 2;
                    break;

                case AC_DEACTIVATE:
                    item->status = IS_DEACTIVATED;
                    break;

                case AC_SOUND_FX:
                case AC_EFFECT:
                    cmd_ptr += 2;
                    break;

                default:
                    break;
                }
            }
        }

        item->anim_num = anim->jump_anim_num;
        item->frame_num = anim->jump_frame_num;
        anim = Item_GetAnim(item);

        if (item->current_anim_state != anim->current_anim_state) {
            item->current_anim_state = anim->current_anim_state;
            item->goal_anim_state = anim->current_anim_state;
        }

        if (item->required_anim_state == item->current_anim_state) {
            item->required_anim_state = 0;
        }
    }

    if (anim->num_commands > 0) {
        const int16_t *cmd_ptr = Anim_GetCommand(anim->command_idx);
        for (int32_t i = 0; i < anim->num_commands; i++) {
            const int16_t cmd = *cmd_ptr++;
            switch (cmd) {
            case AC_MOVE_ORIGIN:
                cmd_ptr += 3;
                break;

            case AC_JUMP_VELOCITY:
                cmd_ptr += 2;
                break;

            case AC_SOUND_FX: {
                const int32_t frame = cmd_ptr[0];
                const SOUND_EFFECT_ID sound_id =
                    ANIM_CMD_PARAM_BITS(cmd_ptr[1]);
                const ANIM_COMMAND_ENVIRONMENT type =
                    ANIM_CMD_ENVIRONMENT_BITS(cmd_ptr[1]);
                cmd_ptr += 2;

                if (item->frame_num != frame) {
                    break;
                }

                if (g_Objects[item->object_id].water_creature) {
                    Sound_Effect(sound_id, &item->pos, SPM_UNDERWATER);
                } else if (item->room_num == NO_ROOM) {
                    item->pos.x = g_LaraItem->pos.x;
                    item->pos.y = g_LaraItem->pos.y - LARA_HEIGHT;
                    item->pos.z = g_LaraItem->pos.z;
                    Sound_Effect(
                        sound_id, &item->pos,
                        item->object_id == O_LARA_HARPOON ? SPM_ALWAYS
                                                          : SPM_NORMAL);
                } else if (g_Rooms[item->room_num].flags & RF_UNDERWATER) {
                    if (type == ACE_ALL || type == ACE_WATER) {
                        Sound_Effect(sound_id, &item->pos, SPM_NORMAL);
                    }
                } else if (type == ACE_ALL || type == ACE_LAND) {
                    Sound_Effect(sound_id, &item->pos, SPM_NORMAL);
                }
                break;
            }

            case AC_EFFECT: {
                const int32_t frame = cmd_ptr[0];
                const int32_t action_id = cmd_ptr[1] & 0x3FFF;
                cmd_ptr += 2;

                if (item->frame_num == frame) {
                    ItemAction_Run(action_id, item);
                }
                break;
            }

            default:
                break;
            }
        }
    }

    if (item->gravity) {
        item->fall_speed += item->fall_speed < FAST_FALL_SPEED ? GRAVITY : 1;
        item->pos.y += item->fall_speed;
    } else {
        int32_t speed = anim->velocity;
        if (anim->acceleration) {
            speed += anim->acceleration * (item->frame_num - anim->frame_base);
        }
        item->speed = speed >> 16;
    }

    item->pos.x += (item->speed * Math_Sin(item->rot.y)) >> W2V_SHIFT;
    item->pos.z += (item->speed * Math_Cos(item->rot.y)) >> W2V_SHIFT;
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

int32_t Item_IsTriggerActive(ITEM *const item)
{
    const bool ok = !(item->flags & IF_REVERSE);

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
    if (item->timer == 0) {
        item->timer = -1;
    }

    return ok;
}

int32_t Item_GetFrames(const ITEM *item, ANIM_FRAME *frmptr[], int32_t *rate)
{
    const ANIM *const anim = Item_GetAnim(item);
    const int32_t cur_frame_num = item->frame_num - anim->frame_base;
    const int32_t key_frame_span = anim->interpolation;
    const int32_t key_frame_shift = cur_frame_num % key_frame_span;
    const int32_t first_key_frame_num = cur_frame_num / key_frame_span;
    const int32_t second_key_frame_num = first_key_frame_num + 1;

    const int32_t numerator = key_frame_shift;
    int32_t denominator = key_frame_span;
    if (numerator != 0) {
        // TODO: ??
        const int32_t second_key_frame_num2 =
            (cur_frame_num / key_frame_span + 1) * key_frame_span;
        if (second_key_frame_num2 > anim->frame_end) {
            denominator += anim->frame_end - second_key_frame_num2;
        }
    }

    frmptr[0] = &anim->frame_ptr[first_key_frame_num];
    frmptr[1] = &anim->frame_ptr[second_key_frame_num];
    *rate = denominator;
    return numerator;
}

BOUNDS_16 *Item_GetBoundsAccurate(const ITEM *const item)
{
    int32_t rate;
    ANIM_FRAME *frmptr[2];
    const int32_t frac = Item_GetFrames(item, frmptr, &rate);
    if (!frac) {
        return &frmptr[0]->bounds;
    }

#define CALC(target, b1, b2, prop)                                             \
    target->prop = (b1)->prop + ((((b2)->prop - (b1)->prop) * frac) / rate);

    BOUNDS_16 *const result = &m_InterpolatedBounds;
    CALC(result, &frmptr[0]->bounds, &frmptr[1]->bounds, min.x);
    CALC(result, &frmptr[0]->bounds, &frmptr[1]->bounds, max.x);
    CALC(result, &frmptr[0]->bounds, &frmptr[1]->bounds, min.y);
    CALC(result, &frmptr[0]->bounds, &frmptr[1]->bounds, max.y);
    CALC(result, &frmptr[0]->bounds, &frmptr[1]->bounds, min.z);
    CALC(result, &frmptr[0]->bounds, &frmptr[1]->bounds, max.z);
    return result;
}

ANIM_FRAME *Item_GetBestFrame(const ITEM *const item)
{
    ANIM_FRAME *frmptr[2];
    int32_t rate;
    const int32_t frac = Item_GetFrames(item, frmptr, &rate);
    return frmptr[(frac > rate / 2) ? 1 : 0];
}

bool Item_IsNearItem(
    const ITEM *const item, const XYZ_32 *const pos, const int32_t distance)
{
    const XYZ_32 d = {
        .x = pos->x - item->pos.x,
        .y = pos->y - item->pos.y,
        .z = pos->z - item->pos.z,
    };
    if (ABS(d.x) > distance || ABS(d.z) > distance || ABS(d.y) > WALL_L * 3) {
        return false;
    }

    if (SQUARE(d.x) + SQUARE(d.z) > SQUARE(distance)) {
        return false;
    }

    const BOUNDS_16 *const bounds = Item_GetBoundsAccurate(item);
    return d.y >= bounds->min.y && d.y <= bounds->max.y + 100;
}

int32_t Item_GetDistance(const ITEM *const item, const XYZ_32 *const target)
{
    return XYZ_32_GetDistance(&item->pos, target);
}

ITEM *Item_Get(const int16_t item_num)
{
    if (item_num == NO_ITEM) {
        return NULL;
    }
    return &g_Items[item_num];
}

int32_t Item_Explode(
    const int16_t item_num, const int32_t mesh_bits, const int16_t damage)
{
    ITEM *const item = &g_Items[item_num];
    const OBJECT *const object = &g_Objects[item->object_id];

    Output_CalculateLight(
        item->pos.x, item->pos.y, item->pos.z, item->room_num);

    const ANIM_FRAME *const best_frame = Item_GetBestFrame(item);

    Matrix_PushUnit();
    g_MatrixPtr->_03 = 0;
    g_MatrixPtr->_13 = 0;
    g_MatrixPtr->_23 = 0;
    g_MatrixPtr->_23 = 0;

    Matrix_RotYXZ(item->rot.y, item->rot.x, item->rot.z);
    Matrix_TranslateRel16(best_frame->offset);
    Matrix_Rot16(best_frame->mesh_rots[0]);

    // main mesh
    int32_t bit = 1;
    if ((mesh_bits & bit) && (item->mesh_bits & bit)) {
        const int16_t effect_num = Effect_Create(item->room_num);
        if (effect_num != NO_EFFECT) {
            EFFECT *const effect = Effect_Get(effect_num);
            effect->pos.x = item->pos.x + (g_MatrixPtr->_03 >> W2V_SHIFT);
            effect->pos.y = item->pos.y + (g_MatrixPtr->_13 >> W2V_SHIFT);
            effect->pos.z = item->pos.z + (g_MatrixPtr->_23 >> W2V_SHIFT);
            effect->rot.y = (Random_GetControl() - 0x4000) * 2;
            effect->room_num = item->room_num;
            effect->speed = Random_GetControl() >> 8;
            effect->fall_speed = -Random_GetControl() >> 8;
            effect->counter = damage;
            effect->object_id = O_BODY_PART;
            effect->frame_num = object->mesh_idx;
            effect->shade = g_LsAdder - 0x300;
        }
        item->mesh_bits &= ~bit;
    }

    // additional meshes
    const int16_t *extra_rotation = (int16_t *)item->data;
    for (int32_t i = 1; i < object->mesh_count; i++) {
        const ANIM_BONE *const bone = Object_GetBone(object, i - 1);
        if (bone->matrix_pop) {
            Matrix_Pop();
        }
        if (bone->matrix_push) {
            Matrix_Push();
        }

        Matrix_TranslateRel32(bone->pos);
        Matrix_Rot16(best_frame->mesh_rots[i]);

        if (extra_rotation != NULL) {
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

        bit <<= 1;
        if ((mesh_bits & bit) && (item->mesh_bits & bit)) {
            const int16_t effect_num = Effect_Create(item->room_num);
            if (effect_num != NO_EFFECT) {
                EFFECT *const effect = Effect_Get(effect_num);
                effect->pos.x = item->pos.x + (g_MatrixPtr->_03 >> W2V_SHIFT);
                effect->pos.y = item->pos.y + (g_MatrixPtr->_13 >> W2V_SHIFT);
                effect->pos.z = item->pos.z + (g_MatrixPtr->_23 >> W2V_SHIFT);
                effect->rot.y = (Random_GetControl() - 0x4000) * 2;
                effect->room_num = item->room_num;
                effect->speed = Random_GetControl() >> 8;
                effect->fall_speed = -Random_GetControl() >> 8;
                effect->counter = damage;
                effect->object_id = O_BODY_PART;
                effect->frame_num = object->mesh_idx + i;
                effect->shade = g_LsAdder - 0x300;
            }
            item->mesh_bits &= ~bit;
        }
    }

    Matrix_Pop();

    return !(item->mesh_bits & (INT32_MAX >> (31 - object->mesh_count)));
}
