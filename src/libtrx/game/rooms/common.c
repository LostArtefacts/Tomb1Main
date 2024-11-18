#include "game/rooms/common.h"

#include "game/gamebuf.h"
#include "game/items.h"
#include "game/rooms/const.h"
#include "game/rooms/enum.h"

#include <assert.h>
#include <stddef.h>

#define FD_NULL_INDEX 0
#define FD_IS_DONE(t) ((t & 0x8000) == 0x8000)

#define FD_ENTRY_TYPE(t) (t & 0x1F)
#define FD_TRIG_TYPE(t) ((t & 0x7F00) >> 8)
#define FD_TRIG_TIMER(t) (t & 0xFF)
#define FD_TRIG_ONE_SHOT(t) ((t & 0x100) == 0x100)
#define FD_TRIG_MASK(t) (t & 0x3E00)
#define FD_TRIG_CMD_TYPE(t) ((t & 0x7C00) >> 10)
#define FD_TRIG_CMD_ARG(t) (t & 0x3FF)
#define FD_TRIG_CAM_GLIDE(t) ((t & 0x3E00) >> 6)

#if TR_VERSION == 2
    #define FD_LADDER_TYPE(t) ((t & 0x7F00) >> 8)
#endif

static const int16_t *M_ReadTrigger(
    const int16_t *data, int16_t fd_entry, SECTOR *sector);

static const int16_t *M_ReadTrigger(
    const int16_t *data, const int16_t fd_entry, SECTOR *const sector)
{
    assert(sector->trigger == NULL);
    TRIGGER *const trigger = GameBuf_Alloc(sizeof(TRIGGER), GBUF_FLOOR_DATA);
    sector->trigger = trigger;

    const int16_t trig_setup = *data++;
    trigger->type = FD_TRIG_TYPE(fd_entry);
    trigger->timer = FD_TRIG_TIMER(trig_setup);
    trigger->one_shot = FD_TRIG_ONE_SHOT(trig_setup);
    trigger->mask = FD_TRIG_MASK(trig_setup);
    trigger->item_index = NO_ITEM;
    trigger->command_count = 0;

    if (trigger->type == TT_SWITCH || trigger->type == TT_KEY
        || trigger->type == TT_PICKUP) {
        const int16_t item_data = *data++;
        trigger->item_index = FD_TRIG_CMD_ARG(item_data);
        if (FD_IS_DONE(item_data)) {
            return data;
        }
    }

    const int16_t *command_data = data;
    while (true) {
        trigger->command_count++;
        int16_t command = *data++;
        if (FD_TRIG_CMD_TYPE(command) == TO_CAMERA) {
            command = *data++;
        }
        if (FD_IS_DONE(command)) {
            break;
        }
    }

    trigger->commands = GameBuf_Alloc(
        sizeof(TRIGGER_CMD) * trigger->command_count, GBUF_FLOOR_DATA);
    for (int32_t i = 0; i < trigger->command_count; i++) {
        int16_t command = *command_data++;
        TRIGGER_CMD *const cmd = &trigger->commands[i];
        cmd->type = FD_TRIG_CMD_TYPE(command);

        if (cmd->type == TO_CAMERA) {
            TRIGGER_CAMERA_DATA *const cam_data =
                GameBuf_Alloc(sizeof(TRIGGER_CAMERA_DATA), GBUF_FLOOR_DATA);
            cmd->parameter = (void *)cam_data;
            cam_data->camera_num = FD_TRIG_CMD_ARG(command);

            command = *command_data++;
            cam_data->timer = FD_TRIG_TIMER(command);
            cam_data->glide = FD_TRIG_CAM_GLIDE(command);
            cam_data->one_shot = FD_TRIG_ONE_SHOT(command);
        } else {
            cmd->parameter = (void *)(intptr_t)FD_TRIG_CMD_ARG(command);
        }
    }

    return data;
}

void Room_ParseFloorData(const int16_t *floor_data)
{
    for (int32_t i = 0; i < Room_GetTotalCount(); i++) {
        const ROOM *const room = Room_Get(i);
        for (int32_t j = 0; j < room->size.x * room->size.z; j++) {
            SECTOR *const sector = &room->sectors[j];
            Room_PopulateSectorData(
                &room->sectors[j], floor_data, sector->idx, FD_NULL_INDEX);
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
    sector->trigger = NULL;
#if TR_VERSION == 2
    sector->ladder = LADDER_NONE;
#endif

    if (start_index == null_index) {
        return;
    }

    const int16_t *data = &floor_data[start_index];
    int16_t fd_entry;
    do {
        fd_entry = *data++;

        switch (FD_ENTRY_TYPE(fd_entry)) {
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

        case FT_TRIGGER:
            data = M_ReadTrigger(data, fd_entry, sector);
            break;

#if TR_VERSION >= 2
        case FT_CLIMB:
            sector->ladder = (LADDER_DIRECTION)FD_LADDER_TYPE(fd_entry);
            break;
#endif

        default:
            break;
        }
    } while (!FD_IS_DONE(fd_entry));
}

int32_t Room_GetAdjoiningRooms(
    int16_t init_room_num, int16_t out_room_nums[],
    const int32_t max_room_num_count)
{
    int32_t count = 0;
    if (max_room_num_count >= 1) {
        out_room_nums[count++] = init_room_num;
    }

    const PORTALS *const portals = Room_Get(init_room_num)->portals;
    if (portals != NULL) {
        for (int32_t i = 0; i < portals->count; i++) {
            const int16_t room_num = portals->portal[i].room_num;
            out_room_nums[count++] = room_num;
        }
    }

    return count;
}
