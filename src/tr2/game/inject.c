#include "game/inject.h"

#include "config.h"
#include "game/room.h"
#include "global/utils.h"

#include <libtrx/benchmark.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>
#include <libtrx/virtual_file.h>

#define INJECTION_MAGIC MKTAG('T', '2', 'X', 'J')
#define INJECTION_CURRENT_VERSION 1
#define NULL_FD_INDEX ((uint16_t)(-1))

typedef enum {
    INJ_VERSION_1 = 1,
} INJECTION_VERSION;

typedef enum {
    INJ_GENERAL = 0,
    INJ_FLOOR_DATA = 4,
} INJECTION_TYPE;

typedef enum {
    FET_TRIGGER_PARAM = 0,
    FET_MUSIC_ONESHOT = 1,
    FET_FD_INSERT = 2,
    FET_ROOM_SHIFT = 3,
    FET_TRIGGER_ITEM = 4,
} FLOOR_EDIT_TYPE;

typedef struct {
    VFILE *fp;
    INJECTION_VERSION version;
    INJECTION_TYPE type;
    INJECTION_INFO *info;
    bool relevant;
} INJECTION;

static int32_t m_NumInjections = 0;
static INJECTION *m_Injections = NULL;

static void M_LoadFromFile(INJECTION *injection, const char *filename);

static void M_FloorDataEdits(const INJECTION *injection);
static void M_TriggerParameterChange(
    const INJECTION *injection, const SECTOR *sector);
static void M_SetMusicOneShot(const SECTOR *sector);
static void M_InsertFloorData(const INJECTION *injection, SECTOR *sector);
static void M_RoomShift(const INJECTION *injection, int16_t room_num);

static void M_LoadFromFile(INJECTION *const injection, const char *filename)
{
    injection->relevant = false;
    injection->info = NULL;

    VFILE *const fp = VFile_CreateFromPath(filename);
    injection->fp = fp;
    if (fp == NULL) {
        LOG_WARNING("Could not open %s", filename);
        return;
    }

    const uint32_t magic = VFile_ReadU32(fp);
    if (magic != INJECTION_MAGIC) {
        LOG_WARNING("Invalid injection magic in %s", filename);
        return;
    }

    injection->version = VFile_ReadS32(fp);
    if (injection->version < INJ_VERSION_1
        || injection->version > INJECTION_CURRENT_VERSION) {
        LOG_WARNING(
            "%s uses unsupported version %d", filename, injection->version);
        return;
    }

    injection->type = VFile_ReadS32(fp);

    switch (injection->type) {
    case INJ_GENERAL:
        injection->relevant = true;
        break;
    case INJ_FLOOR_DATA:
        injection->relevant = g_Config.gameplay.fix_floor_data_issues;
        break;
    default:
        LOG_WARNING("%s is of unknown type %d", filename, injection->type);
        break;
    }

    if (!injection->relevant) {
        return;
    }

    injection->info = Memory_Alloc(sizeof(INJECTION_INFO));
    INJECTION_INFO *const info = injection->info;

    info->floor_edit_count = VFile_ReadS32(fp);

    LOG_INFO("%s queued for injection", filename);
}

static void M_FloorDataEdits(const INJECTION *const injection)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    INJECTION_INFO *const inj_info = injection->info;
    VFILE *const fp = injection->fp;

    for (int32_t i = 0; i < inj_info->floor_edit_count; i++) {
        const int16_t room_num = VFile_ReadS16(fp);
        const uint16_t x = VFile_ReadU16(fp);
        const uint16_t z = VFile_ReadU16(fp);
        const int32_t fd_edit_count = VFile_ReadS32(fp);

        // Verify that the given room and coordinates are accurate.
        // Individual FD functions must check that sector is actually set.
        const ROOM *room = NULL;
        SECTOR *sector = NULL;
        if (room_num < 0 || room_num >= Room_GetTotalCount()) {
            LOG_WARNING("Room index %d is invalid", room_num);
        } else {
            room = Room_Get(room_num);
            if (x >= room->size.x || z >= room->size.z) {
                LOG_WARNING(
                    "Sector [%d,%d] is invalid for room %d", x, z, room_num);
            } else {
                sector = &room->sectors[room->size.z * x + z];
            }
        }

        for (int32_t j = 0; j < fd_edit_count; j++) {
            const FLOOR_EDIT_TYPE edit_type = VFile_ReadS32(fp);
            switch (edit_type) {
            case FET_TRIGGER_PARAM:
                M_TriggerParameterChange(injection, sector);
                break;
            case FET_MUSIC_ONESHOT:
                M_SetMusicOneShot(sector);
                break;
            case FET_FD_INSERT:
                M_InsertFloorData(injection, sector);
                break;
            case FET_ROOM_SHIFT:
                M_RoomShift(injection, room_num);
                break;
            case FET_TRIGGER_ITEM:
                LOG_WARNING("Item injection is not currently supported");
                break;
            default:
                LOG_WARNING("Unknown floor data edit type: %d", edit_type);
                break;
            }
        }
    }

    Benchmark_End(benchmark, NULL);
}

static void M_TriggerParameterChange(
    const INJECTION *const injection, const SECTOR *const sector)
{
    VFILE *const fp = injection->fp;

    const uint8_t cmd_type = VFile_ReadU8(fp);
    const int16_t old_param = VFile_ReadS16(fp);
    const int16_t new_param = VFile_ReadS16(fp);

    if (sector == NULL || sector->trigger == NULL) {
        return;
    }

    // If we can find an action item for the given sector that matches
    // the command type and old (current) parameter, change it to the
    // new parameter.
    for (int32_t i = 0; i < sector->trigger->command_count; i++) {
        TRIGGER_CMD *const cmd = &sector->trigger->commands[i];
        if (cmd->type != cmd_type) {
            continue;
        }

        if (cmd->type == TO_CAMERA) {
            TRIGGER_CAMERA_DATA *const cam_data =
                (TRIGGER_CAMERA_DATA *)cmd->parameter;
            if (cam_data->camera_num == old_param) {
                cam_data->camera_num = new_param;
                break;
            }
        } else {
            if ((int16_t)(intptr_t)cmd->parameter == old_param) {
                cmd->parameter = (void *)(intptr_t)new_param;
                break;
            }
        }
    }
}

static void M_SetMusicOneShot(const SECTOR *const sector)
{
    if (sector == NULL || sector->trigger == NULL) {
        return;
    }

    for (int32_t i = 0; i < sector->trigger->command_count; i++) {
        const TRIGGER_CMD *const cmd = &sector->trigger->commands[i];
        if (cmd->type == TO_CD) {
            sector->trigger->one_shot = true;
            break;
        }
    }
}

static void M_InsertFloorData(
    const INJECTION *const injection, SECTOR *const sector)
{
    VFILE *const fp = injection->fp;

    const int32_t data_length = VFile_ReadS32(fp);

    int16_t data[data_length];
    VFile_Read(fp, data, sizeof(int16_t) * data_length);

    if (sector == NULL) {
        return;
    }

    // This will reset all FD properties in the sector based on the raw data
    // imported. We pass a dummy null index to allow it to read from the
    // beginning of the array.
    Room_PopulateSectorData(sector, data, 0, NULL_FD_INDEX);
}

static void M_RoomShift(
    const INJECTION *const injection, const int16_t room_num)
{
    VFILE *const fp = injection->fp;

    const uint32_t x_shift = ROUND_TO_SECTOR(VFile_ReadU32(fp));
    const uint32_t z_shift = ROUND_TO_SECTOR(VFile_ReadU32(fp));
    const int32_t y_shift = ROUND_TO_CLICK(VFile_ReadS32(fp));

    ROOM *const room = Room_Get(room_num);
    room->pos.x += x_shift;
    room->pos.z += z_shift;
    room->min_floor += y_shift;
    room->max_ceiling += y_shift;

    // Move any items in the room to match.
    for (int32_t i = 0; i < Item_GetTotalCount(); i++) {
        ITEM *const item = Item_Get(i);
        if (item->room_num != room_num) {
            continue;
        }

        item->pos.x += x_shift;
        item->pos.y += y_shift;
        item->pos.z += z_shift;
    }

    if (y_shift == 0) {
        return;
    }

    // Update the sector floor and ceiling heights to match.
    for (int32_t i = 0; i < room->size.z * room->size.x; i++) {
        SECTOR *const sector = &room->sectors[i];
        if (sector->floor.height == NO_HEIGHT
            || sector->ceiling.height == NO_HEIGHT) {
            continue;
        }

        sector->floor.height += y_shift;
        sector->ceiling.height += y_shift;
    }

    // Update vertex Y values to match; x and z are room-relative.
    int16_t *data_ptr = room->data;
    const int16_t vertex_count = *data_ptr++;
    for (int32_t i = 0; i < vertex_count; i++) {
        *(data_ptr + (i * 4) + 1) += y_shift;
    }
}

void Inject_Init(const int injection_count, char *filenames[])
{
    m_NumInjections = injection_count;
    if (m_NumInjections == 0) {
        return;
    }

    BENCHMARK *const benchmark = Benchmark_Start();

    m_Injections = Memory_Alloc(sizeof(INJECTION) * m_NumInjections);
    for (int32_t i = 0; i < m_NumInjections; i++) {
        M_LoadFromFile(&m_Injections[i], filenames[i]);
    }

    Benchmark_End(benchmark, NULL);
}

void Inject_AllInjections(void)
{
    if (m_NumInjections == 0) {
        return;
    }

    BENCHMARK *const benchmark = Benchmark_Start();

    for (int32_t i = 0; i < m_NumInjections; i++) {
        const INJECTION *const injection = &m_Injections[i];
        if (!injection->relevant) {
            continue;
        }

        M_FloorDataEdits(injection);
    }

    Benchmark_End(benchmark, NULL);
}

void Inject_Cleanup(void)
{
    if (m_NumInjections == 0) {
        return;
    }

    BENCHMARK *const benchmark = Benchmark_Start();

    for (int32_t i = 0; i < m_NumInjections; i++) {
        INJECTION *const injection = &m_Injections[i];
        VFile_Close(injection->fp);
        Memory_FreePointer(&injection->info);
    }

    Memory_FreePointer(&m_Injections);
    Benchmark_End(benchmark, NULL);
}
