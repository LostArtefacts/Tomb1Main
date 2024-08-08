#include "game/room.h"

#include "game/camera.h"
#include "game/items.h"
#include "game/lot.h"
#include "game/music.h"
#include "game/objects/common.h"
#include "game/objects/general/bridge.h"
#include "game/objects/general/keyhole.h"
#include "game/objects/general/pickup.h"
#include "game/objects/general/switch.h"
#include "game/objects/traps/lava.h"
#include "game/shell.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#include <stddef.h>

#define NEG_TILT(T, H) ((T * (H & (WALL_L - 1))) >> 2)
#define POS_TILT(T, H) ((T * ((WALL_L - 1 - H) & (WALL_L - 1))) >> 2)

int16_t *g_TriggerIndex = NULL;
int32_t g_FlipTimer = 0;
int32_t g_FlipEffect = -1;
int32_t g_FlipStatus = 0;
int32_t g_FlipMapTable[MAX_FLIP_MAPS] = { 0 };

static void Room_TriggerMusicTrack(int16_t track, int16_t flags, int16_t type);
static void Room_AddFlipItems(ROOM_INFO *r);
static void Room_RemoveFlipItems(ROOM_INFO *r);

static int16_t Room_GetFloorTiltHeight(
    const SECTOR_INFO *sector, const int32_t x, const int32_t z);
static int16_t Room_GetCeilingTiltHeight(
    const SECTOR_INFO *sector, const int32_t x, const int32_t z);

static void Room_PopulateSectorData(
    SECTOR_INFO *sector, const int16_t *floor_data);

static void Room_TriggerMusicTrack(int16_t track, int16_t flags, int16_t type)
{
    if (track == MX_UNUSED_0 && type == TT_ANTIPAD) {
        Music_Stop();
        return;
    }

    if (track <= MX_UNUSED_1 || track >= MAX_CD_TRACKS) {
        return;
    }

    // handle g_Lara gym routines
    switch (track) {
    case MX_GYM_HINT_03:
        if ((g_MusicTrackFlags[track] & IF_ONESHOT)
            && g_LaraItem->current_anim_state == LS_JUMP_UP) {
            track = MX_GYM_HINT_04;
        }
        break;

    case MX_GYM_HINT_12:
        if (g_LaraItem->current_anim_state != LS_HANG) {
            return;
        }
        break;

    case MX_GYM_HINT_16:
        if (g_LaraItem->current_anim_state != LS_HANG) {
            return;
        }
        break;

    case MX_GYM_HINT_17:
        if ((g_MusicTrackFlags[track] & IF_ONESHOT)
            && g_LaraItem->current_anim_state == LS_HANG) {
            track = MX_GYM_HINT_18;
        }
        break;

    case MX_GYM_HINT_24:
        if (g_LaraItem->current_anim_state != LS_SURF_TREAD) {
            return;
        }
        break;

    case MX_GYM_HINT_25:
        if (g_MusicTrackFlags[track] & IF_ONESHOT) {
            static int16_t gym_completion_counter = 0;
            gym_completion_counter++;
            if (gym_completion_counter == LOGIC_FPS * 4) {
                g_LevelComplete = true;
                gym_completion_counter = 0;
            }
        } else if (g_LaraItem->current_anim_state != LS_WATER_OUT) {
            return;
        }
        break;
    }
    // end of g_Lara gym routines

    if (g_MusicTrackFlags[track] & IF_ONESHOT) {
        return;
    }

    if (type == TT_SWITCH) {
        g_MusicTrackFlags[track] ^= flags & IF_CODE_BITS;
    } else if (type == TT_ANTIPAD) {
        g_MusicTrackFlags[track] &= -1 - (flags & IF_CODE_BITS);
    } else if (flags & IF_CODE_BITS) {
        g_MusicTrackFlags[track] |= flags & IF_CODE_BITS;
    }

    if ((g_MusicTrackFlags[track] & IF_CODE_BITS) == IF_CODE_BITS) {
        if (flags & IF_ONESHOT) {
            g_MusicTrackFlags[track] |= IF_ONESHOT;
        }
        Music_Play(track);
    } else {
        Music_StopTrack(track);
    }
}

static void Room_AddFlipItems(ROOM_INFO *r)
{
    for (int16_t item_num = r->item_number; item_num != NO_ITEM;
         item_num = g_Items[item_num].next_item) {
        ITEM_INFO *item = &g_Items[item_num];

        switch (item->object_number) {
        case O_MOVABLE_BLOCK:
        case O_MOVABLE_BLOCK2:
        case O_MOVABLE_BLOCK3:
        case O_MOVABLE_BLOCK4:
            Room_AlterFloorHeight(item, -WALL_L);
            break;

        case O_SLIDING_PILLAR:
            Room_AlterFloorHeight(item, -WALL_L * 2);
            break;

        default:
            break;
        }
    }
}

static void Room_RemoveFlipItems(ROOM_INFO *r)
{
    for (int16_t item_num = r->item_number; item_num != NO_ITEM;
         item_num = g_Items[item_num].next_item) {
        ITEM_INFO *item = &g_Items[item_num];

        switch (item->object_number) {
        case O_MOVABLE_BLOCK:
        case O_MOVABLE_BLOCK2:
        case O_MOVABLE_BLOCK3:
        case O_MOVABLE_BLOCK4:
            Room_AlterFloorHeight(item, WALL_L);
            break;

        case O_SLIDING_PILLAR:
            Room_AlterFloorHeight(item, WALL_L * 2);
            break;

        default:
            break;
        }
    }
}

int16_t Room_GetTiltType(
    const SECTOR_INFO *sector, int32_t x, int32_t y, int32_t z)
{
    ROOM_INFO *r;

    while (sector->pit_room != NO_ROOM) {
        r = &g_RoomInfo[sector->pit_room];
        sector = &r->sectors
                      [((z - r->z) >> WALL_SHIFT)
                       + ((x - r->x) >> WALL_SHIFT) * r->z_size];
    }

    if ((y + STEP_L * 2) < sector->floor.height) {
        return 0;
    }

    return sector->floor.tilt;
}

int32_t Room_FindGridShift(int32_t src, int32_t dst)
{
    int32_t srcw = src >> WALL_SHIFT;
    int32_t dstw = dst >> WALL_SHIFT;
    if (srcw == dstw) {
        return 0;
    }

    src &= WALL_L - 1;
    if (dstw > srcw) {
        return WALL_L - (src - 1);
    } else {
        return -(src + 1);
    }
}

void Room_GetNearByRooms(
    int32_t x, int32_t y, int32_t z, int32_t r, int32_t h, int16_t room_num)
{
    g_RoomsToDrawCount = 0;
    if (g_RoomsToDrawCount + 1 < MAX_ROOMS_TO_DRAW) {
        g_RoomsToDraw[g_RoomsToDrawCount++] = room_num;
    }
    Room_GetNewRoom(x + r, y, z + r, room_num);
    Room_GetNewRoom(x - r, y, z + r, room_num);
    Room_GetNewRoom(x + r, y, z - r, room_num);
    Room_GetNewRoom(x - r, y, z - r, room_num);
    Room_GetNewRoom(x + r, y - h, z + r, room_num);
    Room_GetNewRoom(x - r, y - h, z + r, room_num);
    Room_GetNewRoom(x + r, y - h, z - r, room_num);
    Room_GetNewRoom(x - r, y - h, z - r, room_num);
}

void Room_GetNewRoom(int32_t x, int32_t y, int32_t z, int16_t room_num)
{
    Room_GetSector(x, y, z, &room_num);

    for (int i = 0; i < g_RoomsToDrawCount; i++) {
        int16_t drawn_room = g_RoomsToDraw[i];
        if (drawn_room == room_num) {
            return;
        }
    }

    if (g_RoomsToDrawCount + 1 < MAX_ROOMS_TO_DRAW) {
        g_RoomsToDraw[g_RoomsToDrawCount++] = room_num;
    }
}

SECTOR_INFO *Room_GetSector(int32_t x, int32_t y, int32_t z, int16_t *room_num)
{
    int16_t data;
    SECTOR_INFO *sector;
    const ROOM_INFO *r = &g_RoomInfo[*room_num];
    do {
        int32_t z_sector = (z - r->z) >> WALL_SHIFT;
        int32_t x_sector = (x - r->x) >> WALL_SHIFT;

        if (z_sector <= 0) {
            z_sector = 0;
            if (x_sector < 1) {
                x_sector = 1;
            } else if (x_sector > r->x_size - 2) {
                x_sector = r->x_size - 2;
            }
        } else if (z_sector >= r->z_size - 1) {
            z_sector = r->z_size - 1;
            if (x_sector < 1) {
                x_sector = 1;
            } else if (x_sector > r->x_size - 2) {
                x_sector = r->x_size - 2;
            }
        } else if (x_sector < 0) {
            x_sector = 0;
        } else if (x_sector >= r->x_size) {
            x_sector = r->x_size - 1;
        }

        sector = &r->sectors[z_sector + x_sector * r->z_size];
        if (!sector->index) {
            break;
        }

        data = Room_GetDoor(sector);
        if (data != NO_ROOM) {
            *room_num = data;
            r = &g_RoomInfo[data];
        }
    } while (data != NO_ROOM);

    if (y >= sector->floor.height) {
        do {
            if (sector->pit_room == NO_ROOM) {
                break;
            }

            *room_num = sector->pit_room;

            r = &g_RoomInfo[sector->pit_room];
            const int32_t z_sector = (z - r->z) >> WALL_SHIFT;
            const int32_t x_sector = (x - r->x) >> WALL_SHIFT;
            sector = &r->sectors[z_sector + x_sector * r->z_size];
        } while (y >= sector->floor.height);
    } else if (y < sector->ceiling.height) {
        do {
            if (sector->sky_room == NO_ROOM) {
                break;
            }

            *room_num = sector->sky_room;

            r = &g_RoomInfo[sector->sky_room];
            const int32_t z_sector = (z - r->z) >> WALL_SHIFT;
            const int32_t x_sector = (x - r->x) >> WALL_SHIFT;
            sector = &r->sectors[z_sector + x_sector * r->z_size];
        } while (y < sector->ceiling.height);
    }

    return sector;
}

int16_t Room_GetCeiling(
    const SECTOR_INFO *sector, int32_t x, int32_t y, int32_t z)
{
    int16_t *data;
    int16_t type;
    int16_t trigger;

    const SECTOR_INFO *sky_sector = sector;
    while (sky_sector->sky_room != NO_ROOM) {
        const ROOM_INFO *const r = &g_RoomInfo[sky_sector->sky_room];
        const int32_t z_sector = (z - r->z) >> WALL_SHIFT;
        const int32_t x_sector = (x - r->x) >> WALL_SHIFT;
        sky_sector = &r->sectors[z_sector + x_sector * r->z_size];
    }

    int16_t height = Room_GetCeilingTiltHeight(sky_sector, x, z);

    while (sector->pit_room != NO_ROOM) {
        const ROOM_INFO *const r = &g_RoomInfo[sector->pit_room];
        const int32_t z_sector = (z - r->z) >> WALL_SHIFT;
        const int32_t x_sector = (x - r->x) >> WALL_SHIFT;
        sector = &r->sectors[z_sector + x_sector * r->z_size];
    }

    if (!sector->index) {
        return height;
    }

    data = &g_FloorData[sector->index];
    do {
        type = *data++;

        switch (type & DATA_TYPE) {
        case FT_DOOR:
        case FT_TILT:
        case FT_ROOF:
            data++;
            break;

        case FT_LAVA:
            break;

        case FT_TRIGGER:
            data++;
            do {
                trigger = *data++;
                if (TRIG_BITS(trigger) != TO_OBJECT) {
                    if (TRIG_BITS(trigger) == TO_CAMERA) {
                        trigger = *data++;
                    }
                } else {
                    ITEM_INFO *item = &g_Items[trigger & VALUE_BITS];
                    OBJECT_INFO *object = &g_Objects[item->object_number];
                    if (object->ceiling_height_func) {
                        height =
                            object->ceiling_height_func(item, x, y, z, height);
                    }
                }
            } while (!(trigger & END_BIT));
            break;

        default:
            Shell_ExitSystem("GetCeiling(): Unknown type");
            break;
        }
    } while (!(type & END_BIT));

    return height;
}

int16_t Room_GetDoor(const SECTOR_INFO *sector)
{
    if (!sector->index) {
        return NO_ROOM;
    }

    int16_t *data = &g_FloorData[sector->index];
    int16_t type = *data++;

    if (type == FT_TILT) {
        data++;
        type = *data++;
    }

    if (type == FT_ROOF) {
        data++;
        type = *data++;
    }

    if ((type & DATA_TYPE) == FT_DOOR) {
        return *data;
    }
    return NO_ROOM;
}

int16_t Room_GetHeight(
    const SECTOR_INFO *sector, int32_t x, int32_t y, int32_t z)
{
    g_HeightType = HT_WALL;
    while (sector->pit_room != NO_ROOM) {
        const ROOM_INFO *const r = &g_RoomInfo[sector->pit_room];
        const int32_t z_sector = (z - r->z) >> WALL_SHIFT;
        const int32_t x_sector = (x - r->x) >> WALL_SHIFT;
        sector = &r->sectors[z_sector + x_sector * r->z_size];
    }

    int16_t height = Room_GetFloorTiltHeight(sector, x, z);

    g_TriggerIndex = NULL;

    if (!sector->index) {
        return height;
    }

    int16_t *data = &g_FloorData[sector->index];
    int16_t type;
    int16_t trigger;
    do {
        type = *data++;

        switch (type & DATA_TYPE) {
        case FT_TILT:
        case FT_ROOF:
        case FT_DOOR:
            data++;
            break;

        case FT_LAVA:
            g_TriggerIndex = data - 1;
            break;

        case FT_TRIGGER:
            if (!g_TriggerIndex) {
                g_TriggerIndex = data - 1;
            }

            data++;
            do {
                trigger = *data++;
                if (TRIG_BITS(trigger) != TO_OBJECT) {
                    if (TRIG_BITS(trigger) == TO_CAMERA) {
                        trigger = *data++;
                    }
                } else {
                    ITEM_INFO *item = &g_Items[trigger & VALUE_BITS];
                    OBJECT_INFO *object = &g_Objects[item->object_number];
                    if (object->floor_height_func) {
                        height =
                            object->floor_height_func(item, x, y, z, height);
                    }
                }
            } while (!(trigger & END_BIT));
            break;

        default:
            Shell_ExitSystem("GetHeight(): Unknown type");
            break;
        }
    } while (!(type & END_BIT));

    return height;
}

static int16_t Room_GetFloorTiltHeight(
    const SECTOR_INFO *sector, const int32_t x, const int32_t z)
{
    int16_t height = sector->floor.height;
    if (sector->floor.tilt == 0) {
        return height;
    }

    const int32_t zoff = sector->floor.tilt >> 8;
    const int32_t xoff = (int8_t)sector->floor.tilt;

    const HEIGHT_TYPE slope_type =
        (ABS(zoff) > 2 || ABS(xoff) > 2) ? HT_BIG_SLOPE : HT_SMALL_SLOPE;
    if (g_ChunkyFlag && slope_type == HT_BIG_SLOPE) {
        return height;
    }

    g_HeightType = slope_type;

    if (zoff < 0) {
        height -= (int16_t)NEG_TILT(zoff, z);
    } else {
        height += (int16_t)POS_TILT(zoff, z);
    }

    if (xoff < 0) {
        height -= (int16_t)NEG_TILT(xoff, x);
    } else {
        height += (int16_t)POS_TILT(xoff, x);
    }

    return height;
}

static int16_t Room_GetCeilingTiltHeight(
    const SECTOR_INFO *sector, const int32_t x, const int32_t z)
{
    int16_t height = sector->ceiling.height;
    if (sector->ceiling.tilt == 0) {
        return height;
    }

    const int32_t zoff = sector->ceiling.tilt >> 8;
    const int32_t xoff = (int8_t)sector->ceiling.tilt;

    if (g_ChunkyFlag && (ABS(zoff) > 2 || ABS(xoff) > 2)) {
        return height;
    }

    if (zoff < 0) {
        height += (int16_t)NEG_TILT(zoff, z);
    } else {
        height -= (int16_t)POS_TILT(zoff, z);
    }

    if (xoff < 0) {
        height += (int16_t)POS_TILT(xoff, x);
    } else {
        height -= (int16_t)NEG_TILT(xoff, x);
    }

    return height;
}

int16_t Room_GetWaterHeight(int32_t x, int32_t y, int32_t z, int16_t room_num)
{
    const ROOM_INFO *r = &g_RoomInfo[room_num];

    int16_t data;
    const SECTOR_INFO *sector;
    int32_t z_sector, x_sector;

    do {
        z_sector = (z - r->z) >> WALL_SHIFT;
        x_sector = (x - r->x) >> WALL_SHIFT;

        if (z_sector <= 0) {
            z_sector = 0;
            if (x_sector < 1) {
                x_sector = 1;
            } else if (x_sector > r->x_size - 2) {
                x_sector = r->x_size - 2;
            }
        } else if (z_sector >= r->z_size - 1) {
            z_sector = r->z_size - 1;
            if (x_sector < 1) {
                x_sector = 1;
            } else if (x_sector > r->x_size - 2) {
                x_sector = r->x_size - 2;
            }
        } else if (x_sector < 0) {
            x_sector = 0;
        } else if (x_sector >= r->x_size) {
            x_sector = r->x_size - 1;
        }

        sector = &r->sectors[z_sector + x_sector * r->z_size];
        data = Room_GetDoor(sector);
        if (data != NO_ROOM) {
            r = &g_RoomInfo[data];
        }
    } while (data != NO_ROOM);

    if (r->flags & RF_UNDERWATER) {
        while (sector->sky_room != NO_ROOM) {
            r = &g_RoomInfo[sector->sky_room];
            if (!(r->flags & RF_UNDERWATER)) {
                break;
            }
            z_sector = (z - r->z) >> WALL_SHIFT;
            x_sector = (x - r->x) >> WALL_SHIFT;
            sector = &r->sectors[z_sector + x_sector * r->z_size];
        }
        return sector->ceiling.height;
    } else {
        while (sector->pit_room != NO_ROOM) {
            r = &g_RoomInfo[sector->pit_room];
            if (r->flags & RF_UNDERWATER) {
                return sector->floor.height;
            }
            z_sector = (z - r->z) >> WALL_SHIFT;
            x_sector = (x - r->x) >> WALL_SHIFT;
            sector = &r->sectors[z_sector + x_sector * r->z_size];
        }
        return NO_HEIGHT;
    }
}

int16_t Room_GetIndexFromPos(const int32_t x, const int32_t y, const int32_t z)
{
    for (int i = 0; i < g_RoomCount; i++) {
        const ROOM_INFO *const room = &g_RoomInfo[i];
        const int32_t x1 = room->x + WALL_L;
        const int32_t x2 = room->x + (room->x_size << WALL_SHIFT) - WALL_L;
        const int32_t y1 = room->max_ceiling;
        const int32_t y2 = room->min_floor;
        const int32_t z1 = room->z + WALL_L;
        const int32_t z2 = room->z + (room->z_size << WALL_SHIFT) - WALL_L;
        if (x >= x1 && x < x2 && y >= y1 && y <= y2 && z >= z1 && z < z2) {
            return i;
        }
    }
    return NO_ROOM;
}

void Room_AlterFloorHeight(ITEM_INFO *item, int32_t height)
{
    if (!height) {
        return;
    }

    int16_t data;
    SECTOR_INFO *sector;
    const ROOM_INFO *r = &g_RoomInfo[item->room_number];

    do {
        int32_t z_sector = (item->pos.z - r->z) >> WALL_SHIFT;
        int32_t x_sector = (item->pos.x - r->x) >> WALL_SHIFT;

        if (z_sector <= 0) {
            z_sector = 0;
            CLAMP(x_sector, 1, r->x_size - 2);
        } else if (z_sector >= r->z_size - 1) {
            z_sector = r->z_size - 1;
            CLAMP(x_sector, 1, r->x_size - 2);
        } else {
            CLAMP(x_sector, 0, r->x_size - 1);
        }

        sector = &r->sectors[z_sector + x_sector * r->z_size];
        data = Room_GetDoor(sector);
        if (data != NO_ROOM) {
            r = &g_RoomInfo[data];
        }
    } while (data != NO_ROOM);

    SECTOR_INFO *sky_sector = sector;
    while (sky_sector->sky_room != NO_ROOM) {
        const ROOM_INFO *const r = &g_RoomInfo[sky_sector->sky_room];
        const int32_t z_sector = (item->pos.z - r->z) >> WALL_SHIFT;
        const int32_t x_sector = (item->pos.x - r->x) >> WALL_SHIFT;
        sky_sector = &r->sectors[z_sector + x_sector * r->z_size];
    }

    while (sector->pit_room != NO_ROOM) {
        const ROOM_INFO *const r = &g_RoomInfo[sector->pit_room];
        const int32_t z_sector = (item->pos.z - r->z) >> WALL_SHIFT;
        const int32_t x_sector = (item->pos.x - r->x) >> WALL_SHIFT;
        sector = &r->sectors[z_sector + x_sector * r->z_size];
    }

    if (sector->floor.height != NO_HEIGHT) {
        sector->floor.height += ROUND_TO_CLICK(height);
        if (sector->floor.height == sky_sector->ceiling.height) {
            sector->floor.height = NO_HEIGHT;
        }
    } else {
        sector->floor.height =
            sky_sector->ceiling.height + ROUND_TO_CLICK(height);
    }

    if (g_Boxes[sector->box].overlap_index & BLOCKABLE) {
        if (height < 0) {
            g_Boxes[sector->box].overlap_index |= BLOCKED;
        } else {
            g_Boxes[sector->box].overlap_index &= ~BLOCKED;
        }
    }
}

void Room_FlipMap(void)
{
    Sound_StopAmbientSounds();

    for (int i = 0; i < g_RoomCount; i++) {
        ROOM_INFO *r = &g_RoomInfo[i];
        if (r->flipped_room < 0) {
            continue;
        }

        Room_RemoveFlipItems(r);

        ROOM_INFO *flipped = &g_RoomInfo[r->flipped_room];
        ROOM_INFO temp = *r;
        *r = *flipped;
        *flipped = temp;

        r->flipped_room = flipped->flipped_room;
        flipped->flipped_room = -1;

        // XXX: is this really necessary given the assignments above?
        r->item_number = flipped->item_number;
        r->fx_number = flipped->fx_number;

        Room_AddFlipItems(r);
    }

    g_FlipStatus = !g_FlipStatus;
}

void Room_ParseFloorData(const int16_t *floor_data)
{
    for (int32_t i = 0; i < g_RoomCount; i++) {
        const ROOM_INFO *const room = &g_RoomInfo[i];
        for (int32_t j = 0; j < room->x_size * room->z_size; j++) {
            Room_PopulateSectorData(&room->sectors[j], floor_data);
        }
    }
}

static void Room_PopulateSectorData(
    SECTOR_INFO *const sector, const int16_t *floor_data)
{
    sector->floor.tilt = 0;
    sector->ceiling.tilt = 0;

    if (sector->index == 0) {
        return;
    }

    const int16_t *data = &floor_data[sector->index];
    int16_t fd_entry;
    do {
        fd_entry = *data++;

        switch (fd_entry & DATA_TYPE) {
        case FT_TILT:
            sector->floor.tilt = *data++;
            break;

        case FT_ROOF:
            sector->ceiling.tilt = *data++;
            break;

        case FT_DOOR:
            data++; // TODO: (int16_t)portal_room
            break;

        case FT_LAVA:
            break; // TODO: (bool)is_death_sector

        case FT_TRIGGER: {
            // TODO: (TRIGGER *)trigger
            const int16_t trig_setup = *data++;
            const TRIGGER_TYPE trig_type = TRIG_TYPE(fd_entry);
            if (trig_type == TT_SWITCH || trig_type == TT_KEY
                || trig_type == TT_PICKUP) {
                data++; // TODO: (int16_t)item_index
            }

            while (true) {
                int16_t command = *data++;
                if (TRIG_BITS(command) == TO_CAMERA) {
                    command = *data++;
                }
                if (command & END_BIT) {
                    break;
                }
            }

            break;
        }

        default:
            break;
        }
    } while (!(fd_entry & END_BIT));
}

void Room_TestTriggers(int16_t *data, bool heavy)
{
    if (!data) {
        return;
    }

    if ((*data & DATA_TYPE) == FT_LAVA) {
        if (!heavy && Lava_TestFloor(g_LaraItem)) {
            Lava_Burn(g_LaraItem);
        }

        if (*data & END_BIT) {
            return;
        }

        data++;
    }

    int16_t type = (*data++ >> 8) & 0x3F;
    int32_t switch_off = 0;
    int32_t flip = 0;
    int32_t new_effect = -1;
    int16_t flags = *data++;
    int16_t timer = flags & 0xFF;

    if (g_Camera.type != CAM_HEAVY) {
        Camera_RefreshFromTrigger(type, data);
    }

    if (heavy) {
        if (type != TT_HEAVY) {
            return;
        }
    } else {
        switch (type) {
        case TT_SWITCH: {
            int16_t value = *data++ & VALUE_BITS;
            if (!Switch_Trigger(value, timer)) {
                return;
            }
            switch_off = g_Items[value].current_anim_state == LS_RUN;
            break;
        }

        case TT_PAD:
        case TT_ANTIPAD:
            if (g_LaraItem->pos.y != g_LaraItem->floor) {
                return;
            }
            break;

        case TT_KEY: {
            int16_t value = *data++ & VALUE_BITS;
            if (!KeyHole_Trigger(value)) {
                return;
            }
            break;
        }

        case TT_PICKUP: {
            int16_t value = *data++ & VALUE_BITS;
            if (!Pickup_Trigger(value)) {
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
        }
    }

    ITEM_INFO *camera_item = NULL;
    int16_t trigger;
    do {
        trigger = *data++;
        int16_t value = trigger & VALUE_BITS;

        switch (TRIG_BITS(trigger)) {
        case TO_OBJECT: {
            ITEM_INFO *item = &g_Items[value];

            if (item->flags & IF_ONESHOT) {
                break;
            }

            item->timer = timer;
            if (timer != 1) {
                item->timer *= LOGIC_FPS;
            }

            if (type == TT_SWITCH) {
                item->flags ^= flags & IF_CODE_BITS;
            } else if (type == TT_ANTIPAD) {
                item->flags &= -1 - (flags & IF_CODE_BITS);
            } else if (flags & IF_CODE_BITS) {
                item->flags |= flags & IF_CODE_BITS;
            }

            if ((item->flags & IF_CODE_BITS) != IF_CODE_BITS) {
                break;
            }

            if (flags & IF_ONESHOT) {
                item->flags |= IF_ONESHOT;
            }

            if (!item->active) {
                if (g_Objects[item->object_number].intelligent) {
                    if (item->status == IS_NOT_ACTIVE) {
                        item->touch_bits = 0;
                        item->status = IS_ACTIVE;
                        Item_AddActive(value);
                        LOT_EnableBaddieAI(value, 1);
                    } else if (item->status == IS_INVISIBLE) {
                        item->touch_bits = 0;
                        if (LOT_EnableBaddieAI(value, 0)) {
                            item->status = IS_ACTIVE;
                        } else {
                            item->status = IS_INVISIBLE;
                        }
                        Item_AddActive(value);
                    }
                } else {
                    item->touch_bits = 0;
                    item->status = IS_ACTIVE;
                    Item_AddActive(value);
                }
            }
            break;
        }

        case TO_CAMERA: {
            trigger = *data++;
            int16_t camera_flags = trigger;
            int16_t camera_timer = trigger & 0xFF;

            if (g_Camera.fixed[value].flags & IF_ONESHOT) {
                break;
            }

            g_Camera.number = value;

            if (g_Camera.type == CAM_LOOK || g_Camera.type == CAM_COMBAT) {
                break;
            }

            if (type == TT_COMBAT) {
                break;
            }

            if (type == TT_SWITCH && timer && switch_off) {
                break;
            }

            if (g_Camera.number == g_Camera.last && type != TT_SWITCH) {
                break;
            }

            g_Camera.timer = camera_timer;
            if (g_Camera.timer != 1) {
                g_Camera.timer *= LOGIC_FPS;
            }

            if (camera_flags & IF_ONESHOT) {
                g_Camera.fixed[g_Camera.number].flags |= IF_ONESHOT;
            }

            g_Camera.speed = ((camera_flags & IF_CODE_BITS) >> 6) + 1;
            g_Camera.type = heavy ? CAM_HEAVY : CAM_FIXED;
            break;
        }

        case TO_TARGET:
            camera_item = &g_Items[value];
            break;

        case TO_SINK: {
            OBJECT_VECTOR *obvector = &g_Camera.fixed[value];

            if (g_Lara.LOT.required_box != obvector->flags) {
                g_Lara.LOT.target.x = obvector->x;
                g_Lara.LOT.target.y = obvector->y;
                g_Lara.LOT.target.z = obvector->z;
                g_Lara.LOT.required_box = obvector->flags;
            }

            g_Lara.current_active = obvector->data * 6;
            break;
        }

        case TO_FLIPMAP:
            if (g_FlipMapTable[value] & IF_ONESHOT) {
                break;
            }

            if (type == TT_SWITCH) {
                g_FlipMapTable[value] ^= flags & IF_CODE_BITS;
            } else if (flags & IF_CODE_BITS) {
                g_FlipMapTable[value] |= flags & IF_CODE_BITS;
            }

            if ((g_FlipMapTable[value] & IF_CODE_BITS) == IF_CODE_BITS) {
                if (flags & IF_ONESHOT) {
                    g_FlipMapTable[value] |= IF_ONESHOT;
                }

                if (!g_FlipStatus) {
                    flip = 1;
                }
            } else if (g_FlipStatus) {
                flip = 1;
            }
            break;

        case TO_FLIPON:
            if ((g_FlipMapTable[value] & IF_CODE_BITS) == IF_CODE_BITS
                && !g_FlipStatus) {
                flip = 1;
            }
            break;

        case TO_FLIPOFF:
            if ((g_FlipMapTable[value] & IF_CODE_BITS) == IF_CODE_BITS
                && g_FlipStatus) {
                flip = 1;
            }
            break;

        case TO_FLIPEFFECT:
            new_effect = value;
            break;

        case TO_FINISH:
            g_LevelComplete = true;
            break;

        case TO_CD:
            Room_TriggerMusicTrack(value, flags, type);
            break;

        case TO_SECRET:
            if ((g_GameInfo.current[g_CurrentLevel].stats.secret_flags
                 & (1 << value))) {
                break;
            }
            g_GameInfo.current[g_CurrentLevel].stats.secret_flags |= 1 << value;
            Music_Play(MX_SECRET);
            break;
        }
    } while (!(trigger & END_BIT));

    if (camera_item
        && (g_Camera.type == CAM_FIXED || g_Camera.type == CAM_HEAVY)) {
        g_Camera.item = camera_item;
    }

    if (flip) {
        Room_FlipMap();
        if (new_effect != -1) {
            g_FlipEffect = new_effect;
            g_FlipTimer = 0;
        }
    }
}

bool Room_IsOnWalkable(
    const SECTOR_INFO *sector, const int32_t x, const int32_t y,
    const int32_t z, const int32_t room_height)
{
    while (sector->pit_room != NO_ROOM) {
        const ROOM_INFO *const r = &g_RoomInfo[sector->pit_room];
        const int32_t z_sector = (z - r->z) >> WALL_SHIFT;
        const int32_t x_sector = (x - r->x) >> WALL_SHIFT;
        sector = &r->sectors[z_sector + x_sector * r->z_size];
    }

    int16_t height = sector->floor.height;
    bool object_found = false;

    int16_t *floor_data = &g_FloorData[sector->index];
    int16_t type;
    int16_t trigger;
    int16_t trig_flags;
    int16_t trig_type;

    do {
        type = *floor_data++;

        switch (type & DATA_TYPE) {
        case FT_TILT:
        case FT_ROOF:
        case FT_DOOR:
            floor_data++;
            break;

        case FT_LAVA:
            break;

        case FT_TRIGGER:
            trig_flags = *floor_data;
            floor_data++;
            trig_type = (type >> 8) & 0x3F;
            do {
                trigger = *floor_data++;

                if (TRIG_BITS(trigger) == TO_OBJECT) {
                    const int16_t item_num = trigger & VALUE_BITS;
                    ITEM_INFO *item = &g_Items[item_num];
                    OBJECT_INFO *object = &g_Objects[item->object_number];
                    if (object->floor_height_func) {
                        height =
                            object->floor_height_func(item, x, y, z, height);
                        object_found = true;
                    }
                } else if (TRIG_BITS(trigger) == TO_CAMERA) {
                    trigger = *floor_data++;
                }
            } while (!(trigger & END_BIT));
            break;
        }
    } while (!(type & END_BIT));

    return object_found && room_height == height;
}
