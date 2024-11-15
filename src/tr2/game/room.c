#include "game/room.h"

#include "game/box.h"
#include "game/camera.h"
#include "game/gamebuf.h"
#include "game/items.h"
#include "game/lara/misc.h"
#include "game/lot.h"
#include "game/math.h"
#include "game/music.h"
#include "game/objects/general/keyhole.h"
#include "game/objects/general/pickup.h"
#include "game/objects/general/switch.h"
#include "game/shell.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/utils.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#include <assert.h>

#define NULL_FD_INDEX 0 // TODO: move to libtrx and update TR1
#define TRIG_TYPE(T) ((T & 0x3F00) >> 8)
#define NEG_TILT(T, H) ((T * (H & (WALL_L - 1))) >> 2)
#define POS_TILT(T, H) ((T * ((WALL_L - 1 - H) & (WALL_L - 1))) >> 2)

static int16_t M_GetFloorTiltHeight(const SECTOR *sector, int32_t x, int32_t z);
static int16_t M_GetCeilingTiltHeight(
    const SECTOR *sector, int32_t x, int32_t z);

static void M_TriggerMusicTrack(int16_t track, const TRIGGER *trigger);
static bool M_TestLava(const ITEM *item);
static void M_TestSectorTrigger(const ITEM *item, const SECTOR *sector);

static int16_t M_GetFloorTiltHeight(
    const SECTOR *sector, const int32_t x, const int32_t z)
{
    int16_t height = sector->floor.height;
    if (sector->floor.tilt == 0) {
        return height;
    }

    const int32_t z_off = sector->floor.tilt >> 8;
    const int32_t x_off = (int8_t)sector->floor.tilt;

    const HEIGHT_TYPE slope_type =
        (ABS(z_off) > 2 || ABS(x_off) > 2) ? HT_BIG_SLOPE : HT_SMALL_SLOPE;
    if (g_IsChunkyCamera && slope_type == HT_BIG_SLOPE) {
        return height;
    }

    g_HeightType = slope_type;

    if (z_off < 0) {
        height -= (int16_t)NEG_TILT(z_off, z);
    } else {
        height += (int16_t)POS_TILT(z_off, z);
    }

    if (x_off < 0) {
        height -= (int16_t)NEG_TILT(x_off, x);
    } else {
        height += (int16_t)POS_TILT(x_off, x);
    }

    return height;
}

static int16_t M_GetCeilingTiltHeight(
    const SECTOR *sector, const int32_t x, const int32_t z)
{
    int16_t height = sector->ceiling.height;
    if (sector->ceiling.tilt == 0) {
        return height;
    }

    const int32_t z_off = sector->ceiling.tilt >> 8;
    const int32_t x_off = (int8_t)sector->ceiling.tilt;

    if (g_IsChunkyCamera && (ABS(z_off) > 2 || ABS(x_off) > 2)) {
        return height;
    }

    if (z_off < 0) {
        height += (int16_t)NEG_TILT(z_off, z);
    } else {
        height -= (int16_t)POS_TILT(z_off, z);
    }

    if (x_off < 0) {
        height += (int16_t)POS_TILT(x_off, x);
    } else {
        height -= (int16_t)NEG_TILT(x_off, x);
    }

    return height;
}

static void M_TriggerMusicTrack(
    const int16_t track, const TRIGGER *const trigger)
{
    if (track < MX_CUTSCENE_THE_GREAT_WALL || track >= MX_TITLE_THEME) {
        return;
    }

    if (trigger->type != TT_SWITCH) {
        const int32_t code = trigger->mask;
        if (g_MusicTrackFlags[track] & code) {
            return;
        }
        if (trigger->one_shot) {
            g_MusicTrackFlags[track] |= code;
        }
    }

    if (trigger->timer == 0) {
        Music_Play(track, MPM_TRACKED);
        return;
    }

    if (track != Music_GetDelayedTrack()) {
        Music_Play(track, MPM_DELAYED);
        g_MusicTrackFlags[track] = (g_MusicTrackFlags[track] & 0xFF00)
            | ((FRAMES_PER_SECOND * trigger->timer) & 0xFF);
        return;
    }

    int32_t timer = g_MusicTrackFlags[track] & 0xFF;
    if (timer == 0) {
        return;
    }

    timer--;
    if (timer == 0) {
        Music_Play(track, MPM_TRACKED);
    }
    g_MusicTrackFlags[track] =
        (g_MusicTrackFlags[track] & 0xFF00) | (timer & 0xFF);
}

static bool M_TestLava(const ITEM *const item)
{
    if (item->hit_points < 0 || g_Lara.water_status == LWS_CHEAT
        || (g_Lara.water_status == LWS_ABOVE_WATER
            && item->pos.y != item->floor)) {
        return false;
    }

    // OG fix: check if floor index has lava
    int16_t room_num = item->room_num;
    const SECTOR *const sector =
        Room_GetSector(item->pos.x, MAX_HEIGHT, item->pos.z, &room_num);
    return sector->is_death_sector;
}

static void M_TestSectorTrigger(
    const ITEM *const item, const SECTOR *const sector)
{
    const bool is_heavy = item->object_id != O_LARA;
    if (!is_heavy) {
        if (sector->is_death_sector && M_TestLava(item)) {
            Lara_TouchLava((ITEM *)item);
        }

        const LADDER_DIRECTION direction = 1 << Math_GetDirection(item->rot.y);
        g_Lara.climb_status = (sector->ladder & direction) == direction;
    }

    const TRIGGER *const trigger = sector->trigger;
    if (trigger == NULL) {
        return;
    }

    if (g_Camera.type != CAM_HEAVY) {
        Camera_RefreshFromTrigger(trigger);
    }

    ITEM *camera_item = NULL;
    bool switch_off = false;
    bool flip_map = false;
    bool flip_available = false;
    int32_t new_effect = -1;

    if (is_heavy) {
        if (trigger->type != TT_HEAVY) {
            return;
        }
    } else {
        switch (trigger->type) {
        case TT_PAD:
        case TT_ANTIPAD:
            if (item->pos.y != item->floor) {
                return;
            }
            break;

        case TT_SWITCH: {
            if (!Switch_Trigger(trigger->item_index, trigger->timer)) {
                return;
            }
            const ITEM *const switch_item = Item_Get(trigger->item_index);
            switch_off = switch_item->current_anim_state == LS_RUN;
            break;
        }

        case TT_KEY: {
            if (!Keyhole_Trigger(trigger->item_index)) {
                return;
            }
            break;
        }

        case TT_PICKUP: {
            if (!Pickup_Trigger(trigger->item_index)) {
                return;
            }
            break;
        }

        case TT_HEAVY:
        case TT_DUMMY:
            return;

        case TT_COMBAT:
            if (g_Lara.gun_status != LGS_READY) {
                return;
            }
            break;

        default:
            break;
        }
    }

    for (int32_t i = 0; i < trigger->command_count; i++) {
        const TRIGGER_CMD *const cmd = &trigger->commands[i];

        switch (cmd->type) {
        case TO_OBJECT: {
            const int16_t item_num = (int16_t)(intptr_t)cmd->parameter;
            ITEM *const trig_item = Item_Get(item_num);
            if (trig_item->flags & IF_ONE_SHOT) {
                break;
            }

            trig_item->timer = trigger->timer;
            if (trig_item->timer != 1) {
                trig_item->timer *= FRAMES_PER_SECOND;
            }

            if (trigger->type == TT_SWITCH) {
                trig_item->flags ^= trigger->mask;
            } else if (
                trigger->type == TT_ANTIPAD
                || trigger->type == TT_ANTITRIGGER) {
                trig_item->flags &= ~trigger->mask;
                if (trigger->one_shot) {
                    trig_item->flags |= IF_ONE_SHOT;
                }
            } else {
                trig_item->flags |= trigger->mask;
            }

            if ((trig_item->flags & IF_CODE_BITS) != IF_CODE_BITS) {
                break;
            }

            if (trigger->one_shot) {
                trig_item->flags |= IF_ONE_SHOT;
            }

            if (trig_item->active) {
                break;
            }

            const OBJECT *const obj = Object_GetObject(trig_item->object_id);
            if (obj->intelligent) {
                if (trig_item->status == IS_INACTIVE) {
                    trig_item->touch_bits = 0;
                    trig_item->status = IS_ACTIVE;
                    Item_AddActive(item_num);
                    LOT_EnableBaddieAI(item_num, true);
                } else if (trig_item->status == IS_INVISIBLE) {
                    trig_item->touch_bits = 0;
                    if (LOT_EnableBaddieAI(item_num, false)) {
                        trig_item->status = IS_ACTIVE;
                    } else {
                        trig_item->status = IS_INVISIBLE;
                    }
                    Item_AddActive(item_num);
                }
            } else {
                trig_item->touch_bits = 0;
                trig_item->status = IS_ACTIVE;
                Item_AddActive(item_num);
            }

            break;
        }

        case TO_CAMERA: {
            const TRIGGER_CAMERA_DATA *const cam_data =
                (TRIGGER_CAMERA_DATA *)cmd->parameter;
            if (g_Camera.fixed[cam_data->camera_num].flags & IF_ONE_SHOT) {
                break;
            }

            g_Camera.num = cam_data->camera_num;

            if (g_Camera.type == CAM_LOOK || g_Camera.type == CAM_COMBAT) {
                break;
            }

            if (trigger->type == TT_COMBAT) {
                break;
            }

            if (trigger->type == TT_SWITCH && trigger->timer && switch_off) {
                break;
            }

            if (g_Camera.num == g_Camera.last && trigger->type != TT_SWITCH) {
                break;
            }

            g_Camera.timer = FRAMES_PER_SECOND * cam_data->timer;

            if (cam_data->one_shot) {
                g_Camera.fixed[g_Camera.num].flags |= IF_ONE_SHOT;
            }

            g_Camera.speed = cam_data->glide + 1;
            g_Camera.type = is_heavy ? CAM_HEAVY : CAM_FIXED;
            break;
        }

        case TO_SINK: {
            const OBJECT_VECTOR *const object_vector =
                &g_Camera.fixed[(int16_t)(intptr_t)cmd->parameter];

            if (!g_Lara.creature) {
                LOT_EnableBaddieAI(g_Lara.item_num, true);
            }

            g_Lara.creature->lot.target.x = object_vector->x;
            g_Lara.creature->lot.target.y = object_vector->y;
            g_Lara.creature->lot.target.z = object_vector->z;
            g_Lara.creature->lot.required_box = object_vector->flags;
            g_Lara.current_active = object_vector->data * 6;
            break;
        }

        case TO_FLIPMAP: {
            const int16_t flip_slot = (int16_t)(intptr_t)cmd->parameter;
            flip_available = true;

            if (g_FlipMaps[flip_slot] & IF_ONE_SHOT) {
                break;
            }

            if (trigger->type == TT_SWITCH) {
                g_FlipMaps[flip_slot] ^= trigger->mask;
            } else {
                g_FlipMaps[flip_slot] |= trigger->mask;
            }

            if ((g_FlipMaps[flip_slot] & IF_CODE_BITS) == IF_CODE_BITS) {
                if (trigger->one_shot) {
                    g_FlipMaps[flip_slot] |= IF_ONE_SHOT;
                }

                if (!g_FlipStatus) {
                    flip_map = true;
                }
            } else if (g_FlipStatus) {
                flip_map = true;
            }
            break;
        }

        case TO_FLIPON: {
            const int16_t flip_slot = (int16_t)(intptr_t)cmd->parameter;
            flip_available = true;

            if ((g_FlipMaps[flip_slot] & IF_CODE_BITS) == IF_CODE_BITS
                && !g_FlipStatus) {
                flip_map = true;
            }
            break;
        }

        case TO_FLIPOFF: {
            const int16_t flip_slot = (int16_t)(intptr_t)cmd->parameter;
            flip_available = true;

            if ((g_FlipMaps[flip_slot] & IF_CODE_BITS) == IF_CODE_BITS
                && g_FlipStatus) {
                flip_map = true;
            }
            break;
        }

        case TO_TARGET: {
            const int16_t target_num = (int16_t)(intptr_t)cmd->parameter;
            camera_item = Item_Get(target_num);
            break;
        }

        case TO_FINISH:
            g_LevelComplete = true;
            break;

        case TO_FLIPEFFECT:
            new_effect = (int16_t)(intptr_t)cmd->parameter;
            break;

        case TO_CD:
            M_TriggerMusicTrack((int16_t)(intptr_t)cmd->parameter, trigger);
            break;

        case TO_BODY_BAG:
            Item_ClearKilled();
            break;

        default:
            break;
        }
    }

    if (camera_item != NULL
        && (g_Camera.type == CAM_FIXED || g_Camera.type == CAM_HEAVY)) {
        g_Camera.item = camera_item;
    }

    if (flip_map) {
        Room_FlipMap();
    }

    if (new_effect != -1 && (flip_map || !flip_available)) {
        g_FlipEffect = new_effect;
        g_FlipTimer = 0;
    }
}

int16_t Room_GetIndexFromPos(const int32_t x, const int32_t y, const int32_t z)
{
    // TODO: merge this to Room_FindByPos
    const int32_t room_num = Room_FindByPos(x, y, z);
    if (room_num == NO_ROOM_NEG) {
        return NO_ROOM;
    }
    return room_num;
}

int32_t __cdecl Room_FindByPos(
    const int32_t x, const int32_t y, const int32_t z)
{
    for (int32_t i = 0; i < g_RoomCount; i++) {
        const ROOM *const room = &g_Rooms[i];
        const int32_t x1 = room->pos.x + WALL_L;
        const int32_t x2 = room->pos.x + (room->size.x - 1) * WALL_L;
        const int32_t y1 = room->max_ceiling;
        const int32_t y2 = room->min_floor;
        const int32_t z1 = room->pos.z + WALL_L;
        const int32_t z2 = room->pos.z + (room->size.z - 1) * WALL_L;
        if (x >= x1 && x < x2 && y >= y1 && y <= y2 && z >= z1 && z < z2) {
            return i;
        }
    }

    return NO_ROOM_NEG;
}

int32_t __cdecl Room_FindGridShift(int32_t src, const int32_t dst)
{
    const int32_t src_w = src >> WALL_SHIFT;
    const int32_t dst_w = dst >> WALL_SHIFT;
    if (src_w == dst_w) {
        return 0;
    }

    src &= WALL_L - 1;
    if (dst_w > src_w) {
        return WALL_L - (src - 1);
    } else {
        return -(src + 1);
    }
}

void __cdecl Room_GetNearbyRooms(
    const int32_t x, const int32_t y, const int32_t z, const int32_t r,
    const int32_t h, const int16_t room_num)
{
    g_DrawRoomsArray[0] = room_num;
    g_DrawRoomsCount = 1;

    Room_GetNewRoom(r + x, y, r + z, room_num);
    Room_GetNewRoom(x - r, y, r + z, room_num);
    Room_GetNewRoom(r + x, y, z - r, room_num);
    Room_GetNewRoom(x - r, y, z - r, room_num);
    Room_GetNewRoom(r + x, y - h, r + z, room_num);
    Room_GetNewRoom(x - r, y - h, r + z, room_num);
    Room_GetNewRoom(r + x, y - h, z - r, room_num);
    Room_GetNewRoom(x - r, y - h, z - r, room_num);
}

void __cdecl Room_GetNewRoom(
    const int32_t x, const int32_t y, const int32_t z, int16_t room_num)
{
    Room_GetSector(x, y, z, &room_num);

    for (int32_t i = 0; i < g_DrawRoomsCount; i++) {
        if (g_DrawRoomsArray[i] == room_num) {
            return;
        }
    }

    // TODO: fix crash when trying to draw too many rooms
    g_DrawRoomsArray[g_DrawRoomsCount++] = room_num;
}

int16_t __cdecl Room_GetTiltType(
    const SECTOR *sector, const int32_t x, const int32_t y, const int32_t z)
{
    sector = Room_GetPitSector(sector, x, z);

    if ((y + STEP_L * 2) < sector->floor.height) {
        return 0;
    }

    return sector->floor.tilt;
}

SECTOR *Room_GetPitSector(
    const SECTOR *sector, const int32_t x, const int32_t z)
{
    while (sector->portal_room.pit != NO_ROOM) {
        const ROOM *const room = Room_Get(sector->portal_room.pit);
        const int32_t z_sector = (z - room->pos.z) >> WALL_SHIFT;
        const int32_t x_sector = (x - room->pos.x) >> WALL_SHIFT;
        sector = &room->sectors[z_sector + x_sector * room->size.z];
    }

    return (SECTOR *)sector;
}

SECTOR *Room_GetSkySector(
    const SECTOR *sector, const int32_t x, const int32_t z)
{
    while (sector->portal_room.sky != NO_ROOM) {
        const ROOM *const room = Room_Get(sector->portal_room.sky);
        const int32_t z_sector = (z - room->pos.z) >> WALL_SHIFT;
        const int32_t x_sector = (x - room->pos.x) >> WALL_SHIFT;
        sector = &room->sectors[z_sector + x_sector * room->size.z];
    }

    return (SECTOR *)sector;
}

SECTOR *__cdecl Room_GetSector(
    const int32_t x, const int32_t y, const int32_t z, int16_t *const room_num)
{
    SECTOR *sector = NULL;

    while (true) {
        const ROOM *r = &g_Rooms[*room_num];
        int32_t z_sector = (z - r->pos.z) >> WALL_SHIFT;
        int32_t x_sector = (x - r->pos.x) >> WALL_SHIFT;

        if (z_sector <= 0) {
            z_sector = 0;
            if (x_sector < 1) {
                x_sector = 1;
            } else if (x_sector > r->size.x - 2) {
                x_sector = r->size.x - 2;
            }
        } else if (z_sector >= r->size.z - 1) {
            z_sector = r->size.z - 1;
            if (x_sector < 1) {
                x_sector = 1;
            } else if (x_sector > r->size.x - 2) {
                x_sector = r->size.x - 2;
            }
        } else if (x_sector < 0) {
            x_sector = 0;
        } else if (x_sector >= r->size.x) {
            x_sector = r->size.x - 1;
        }

        sector = &r->sectors[z_sector + x_sector * r->size.z];
        if (sector->portal_room.wall == NO_ROOM) {
            break;
        }
        *room_num = sector->portal_room.wall;
    }

    assert(sector != NULL);

    if (y >= sector->floor.height) {
        while (sector->portal_room.pit != NO_ROOM) {
            *room_num = sector->portal_room.pit;
            const ROOM *const r = &g_Rooms[*room_num];
            const int32_t z_sector = ((z - r->pos.z) >> WALL_SHIFT);
            const int32_t x_sector = ((x - r->pos.x) >> WALL_SHIFT);
            sector = &r->sectors[z_sector + x_sector * r->size.z];
            if (y < sector->floor.height) {
                break;
            }
        }
    } else if (y < sector->ceiling.height) {
        while (sector->portal_room.sky != NO_ROOM) {
            *room_num = sector->portal_room.sky;
            const ROOM *const r = &g_Rooms[sector->portal_room.sky];
            const int32_t z_sector = (z - r->pos.z) >> WALL_SHIFT;
            const int32_t x_sector = (x - r->pos.x) >> WALL_SHIFT;
            sector = &r->sectors[z_sector + x_sector * r->size.z];
            if (y >= sector->ceiling.height) {
                break;
            }
        }
    }

    return sector;
}

int32_t __cdecl Room_GetWaterHeight(
    const int32_t x, const int32_t y, const int32_t z, int16_t room_num)
{
    const SECTOR *sector = NULL;
    const ROOM *r = NULL;

    do {
        r = &g_Rooms[room_num];
        int32_t z_sector = (z - r->pos.z) >> WALL_SHIFT;
        int32_t x_sector = (x - r->pos.x) >> WALL_SHIFT;

        if (z_sector <= 0) {
            z_sector = 0;
            if (x_sector < 1) {
                x_sector = 1;
            } else if (x_sector > r->size.x - 2) {
                x_sector = r->size.x - 2;
            }
        } else if (z_sector >= r->size.z - 1) {
            z_sector = r->size.z - 1;
            if (x_sector < 1) {
                x_sector = 1;
            } else if (x_sector > r->size.x - 2) {
                x_sector = r->size.x - 2;
            }
        } else if (x_sector < 0) {
            x_sector = 0;
        } else if (x_sector >= r->size.x) {
            x_sector = r->size.x - 1;
        }

        sector = &r->sectors[z_sector + x_sector * r->size.z];
        room_num = sector->portal_room.wall;
    } while (room_num != NO_ROOM);

    if (r->flags & RF_UNDERWATER) {
        while (sector->portal_room.sky != NO_ROOM) {
            r = &g_Rooms[sector->portal_room.sky];
            if (!(r->flags & RF_UNDERWATER)) {
                break;
            }
            const int32_t z_sector = (z - r->pos.z) >> WALL_SHIFT;
            const int32_t x_sector = (x - r->pos.x) >> WALL_SHIFT;
            sector = &r->sectors[z_sector + x_sector * r->size.z];
        }
        return sector->ceiling.height;
    } else {
        while (sector->portal_room.pit != NO_ROOM) {
            r = &g_Rooms[sector->portal_room.pit];
            if (r->flags & RF_UNDERWATER) {
                return sector->floor.height;
            }
            const int32_t z_sector = (z - r->pos.z) >> WALL_SHIFT;
            const int32_t x_sector = (x - r->pos.x) >> WALL_SHIFT;
            sector = &r->sectors[z_sector + x_sector * r->size.z];
        }
        return NO_HEIGHT;
    }
}

int32_t __cdecl Room_GetHeight(
    const SECTOR *sector, const int32_t x, const int32_t y, const int32_t z)
{
    g_HeightType = 0;

    const SECTOR *const pit_sector = Room_GetPitSector(sector, x, z);
    int32_t height = pit_sector->floor.height;

    if (g_GF_NoFloor && g_GF_NoFloor == height) {
        height = 0x4000;
    } else {
        height = M_GetFloorTiltHeight(pit_sector, x, z);
    }

    if (pit_sector->trigger == NULL) {
        return height;
    }

    for (int32_t i = 0; i < pit_sector->trigger->command_count; i++) {
        const TRIGGER_CMD *const cmd = &pit_sector->trigger->commands[i];
        if (cmd->type != TO_OBJECT) {
            continue;
        }

        const ITEM *const item = Item_Get((int16_t)(intptr_t)cmd->parameter);
        const OBJECT *const object = Object_GetObject(item->object_id);
        if (object->floor) {
            object->floor(item, x, y, z, &height);
        }
    }

    return height;
}

void Room_ParseFloorData(const int16_t *floor_data)
{
    for (int32_t i = 0; i < g_RoomCount; i++) {
        const ROOM *const room = Room_Get(i);
        for (int32_t j = 0; j < room->size.x * room->size.z; j++) {
            SECTOR *const sector = &room->sectors[j];
            Room_PopulateSectorData(
                &room->sectors[j], floor_data, sector->idx, NULL_FD_INDEX);
        }
    }
}

void Room_PopulateSectorData(
    SECTOR *const sector, const int16_t *floor_data, const uint16_t start_index,
    const uint16_t null_index)
{
    sector->floor.tilt = 0;
    sector->ceiling.tilt = 0;
    sector->portal_room.wall = NO_ROOM;
    sector->is_death_sector = false;
    sector->ladder = LADDER_NONE;
    sector->trigger = NULL;

    if (start_index == null_index) {
        return;
    }

    const int16_t *data = &floor_data[sector->idx];
    int16_t fd_entry;
    do {
        fd_entry = *data++;

        switch (FLOORDATA_TYPE(fd_entry)) {
        case FT_TILT:
            sector->floor.tilt = *data++;
            break;

        case FT_ROOF:
            sector->ceiling.tilt = *data++;
            break;

        case FT_DOOR:
            sector->portal_room.wall = *data++;
            break;

        case FT_LAVA:
            sector->is_death_sector = true;
            break;

        case FT_CLIMB:
            sector->ladder = (LADDER_DIRECTION)((fd_entry & 0x7F00) >> 8);
            break;

        case FT_TRIGGER: {
            assert(sector->trigger == NULL);

            TRIGGER *const trigger =
                GameBuf_Alloc(sizeof(TRIGGER), GBUF_FLOOR_DATA);
            sector->trigger = trigger;

            const int16_t trig_setup = *data++;
            trigger->type = TRIG_TYPE(fd_entry);
            trigger->timer = trig_setup & 0xFF;
            trigger->one_shot = trig_setup & IF_ONE_SHOT;
            trigger->mask = trig_setup & IF_CODE_BITS;
            trigger->item_index = NO_ITEM;
            trigger->command_count = 0;

            if (trigger->type == TT_SWITCH || trigger->type == TT_KEY
                || trigger->type == TT_PICKUP) {
                const int16_t item_data = *data++;
                trigger->item_index = TRIGGER_VALUE(item_data);
                if (TRIGGER_IS_END(item_data)) {
                    break;
                }
            }

            const int16_t *command_data = data;
            while (true) {
                trigger->command_count++;
                int16_t command = *data++;
                if (TRIGGER_TYPE(command) == TO_CAMERA) {
                    command = *data++;
                }
                if (TRIGGER_IS_END(command)) {
                    break;
                }
            }

            trigger->commands = GameBuf_Alloc(
                sizeof(TRIGGER_CMD) * trigger->command_count, GBUF_FLOOR_DATA);
            for (int32_t i = 0; i < trigger->command_count; i++) {
                int16_t command = *command_data++;
                TRIGGER_CMD *const cmd = &trigger->commands[i];
                cmd->type = TRIGGER_TYPE(command);

                if (cmd->type == TO_CAMERA) {
                    TRIGGER_CAMERA_DATA *const cam_data = GameBuf_Alloc(
                        sizeof(TRIGGER_CAMERA_DATA), GBUF_FLOOR_DATA);
                    cmd->parameter = (void *)cam_data;
                    cam_data->camera_num = TRIGGER_VALUE(command);

                    command = *command_data++;
                    cam_data->timer = command & 0xFF;
                    cam_data->glide = (command & IF_CODE_BITS) >> 6;
                    cam_data->one_shot = command & IF_ONE_SHOT;
                } else {
                    cmd->parameter = (void *)(intptr_t)TRIGGER_VALUE(command);
                }
            }

            break;
        }

        default:
            break;
        }
    } while (!FLOORDATA_IS_END(fd_entry));
}

void __cdecl Room_Legacy_TestTriggers(const int16_t *fd, bool heavy)
{
    assert(false);
}

void Room_TestTriggers(const ITEM *const item)
{
    int16_t room_num = item->room_num;
    const SECTOR *sector =
        Room_GetSector(item->pos.x, MAX_HEIGHT, item->pos.z, &room_num);

    M_TestSectorTrigger(item, sector);
}

int32_t __cdecl Room_GetCeiling(
    const SECTOR *const sector, const int32_t x, const int32_t y,
    const int32_t z)
{
    const SECTOR *const sky_sector = Room_GetSkySector(sector, x, z);
    int32_t height = M_GetCeilingTiltHeight(sky_sector, x, z);

    const SECTOR *const pit_sector = Room_GetPitSector(sector, x, z);
    if (pit_sector->trigger == NULL) {
        return height;
    }

    for (int32_t i = 0; i < pit_sector->trigger->command_count; i++) {
        const TRIGGER_CMD *const cmd = &pit_sector->trigger->commands[i];
        if (cmd->type != TO_OBJECT) {
            continue;
        }

        const ITEM *const item = Item_Get((int16_t)(intptr_t)cmd->parameter);
        const OBJECT *const object = Object_GetObject(item->object_id);
        if (object->ceiling) {
            object->ceiling(item, x, y, z, &height);
        }
    }

    return height;
}

int16_t __cdecl Room_Legacy_GetDoor(const SECTOR *const sector)
{
    assert(false);
    return NO_ROOM;
}

void __cdecl Room_AlterFloorHeight(const ITEM *const item, const int32_t height)
{
    int16_t room_num = item->room_num;

    SECTOR *const sector =
        Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);
    const SECTOR *ceiling = Room_GetSector(
        item->pos.x, item->pos.y + height - WALL_L, item->pos.z, &room_num);

    if (sector->floor.height == NO_HEIGHT) {
        sector->floor.height = ceiling->ceiling.height + ROUND_TO_CLICK(height);
    } else {
        sector->floor.height += ROUND_TO_CLICK(height);
        if (sector->floor.height == ceiling->ceiling.height) {
            sector->floor.height = NO_HEIGHT;
        }
    }

    BOX_INFO *const box = &g_Boxes[sector->box];
    if (box->overlap_index & BOX_BLOCKABLE) {
        if (height < 0) {
            box->overlap_index |= BOX_BLOCKED;
        } else {
            box->overlap_index &= ~BOX_BLOCKED;
        }
    }
}

bool Room_GetFlipStatus(void)
{
    return g_FlipStatus;
}

void __cdecl Room_FlipMap(void)
{
    for (int32_t i = 0; i < g_RoomCount; i++) {
        ROOM *const r = &g_Rooms[i];
        if (r->flipped_room == NO_ROOM_NEG) {
            continue;
        }

        Room_RemoveFlipItems(r);

        ROOM *const flipped = &g_Rooms[r->flipped_room];
        ROOM temp = *r;
        *r = *flipped;
        *flipped = temp;

        r->flipped_room = flipped->flipped_room;
        flipped->flipped_room = NO_ROOM_NEG;

        // TODO: is this really necessary given the assignments above?
        r->item_num = flipped->item_num;
        r->fx_num = flipped->fx_num;

        Room_AddFlipItems(r);
    }

    g_FlipStatus = !g_FlipStatus;
}

void __cdecl Room_RemoveFlipItems(const ROOM *const r)
{
    int16_t item_num = r->item_num;

    while (item_num != NO_ITEM) {
        ITEM *const item = &g_Items[item_num];

        switch (item->object_id) {
        case O_MOVABLE_BLOCK_1:
        case O_MOVABLE_BLOCK_2:
        case O_MOVABLE_BLOCK_3:
        case O_MOVABLE_BLOCK_4:
            Room_AlterFloorHeight(item, WALL_L);
            break;

        default:
            break;
        }

        if (item->flags & IF_ONE_SHOT && g_Objects[item->object_id].intelligent
            && item->hit_points <= 0) {
            Item_RemoveDrawn(item_num);
            item->flags |= IF_KILLED;
        }

        item_num = item->next_item;
    }
}

void __cdecl Room_AddFlipItems(const ROOM *const r)
{
    int16_t item_num = r->item_num;
    while (item_num != NO_ITEM) {
        const ITEM *const item = &g_Items[item_num];

        switch (item->object_id) {
        case O_MOVABLE_BLOCK_1:
        case O_MOVABLE_BLOCK_2:
        case O_MOVABLE_BLOCK_3:
        case O_MOVABLE_BLOCK_4:
            Room_AlterFloorHeight(item, -WALL_L);
            break;

        default:
            break;
        }

        item_num = item->next_item;
    }
}

void __cdecl Room_Legacy_TriggerMusicTrack(
    const int16_t track, const int16_t flags, const int16_t type)
{
    assert(false);
}

int32_t Room_GetTotalCount(void)
{
    return g_RoomCount;
}

ROOM *Room_Get(const int32_t room_num)
{
    return &g_Rooms[room_num];
}
