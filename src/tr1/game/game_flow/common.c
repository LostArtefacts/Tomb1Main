#include "game/game_flow/common.h"

#include "game/game_flow/vars.h"
#include "global/vars.h"

#include <libtrx/memory.h>

static void M_FreeSequence(GAME_FLOW_SEQUENCE *sequence);

static void M_FreeSequence(GAME_FLOW_SEQUENCE *const sequence)
{
    for (int32_t i = 0; i < sequence->length; i++) {
        GAME_FLOW_SEQUENCE_EVENT *const event = &sequence->events[i];
        switch (event->type) {
        case GFS_LOADING_SCREEN:
        case GFS_DISPLAY_PICTURE:
        case GFS_TOTAL_STATS: {
            GAME_FLOW_DISPLAY_PICTURE_DATA *data = event->data;
            Memory_FreePointer(&data->path);
            Memory_FreePointer(&data);
            break;
        }
        case GFS_PLAY_FMV:
        case GFS_MESH_SWAP:
        case GFS_GIVE_ITEM:
            Memory_FreePointer(&event->data);
            break;
        case GFS_START_GAME:
        case GFS_LOOP_GAME:
        case GFS_LOOP_CINE:
        case GFS_LEVEL_STATS:
        case GFS_EXIT_TO_TITLE:
        case GFS_EXIT_TO_LEVEL:
        case GFS_EXIT_TO_CINE:
        case GFS_SET_CAM_ANGLE:
        case GFS_FLIP_MAP:
        case GFS_PLAY_SYNCED_AUDIO:
        case GFS_REMOVE_GUNS:
        case GFS_REMOVE_SCIONS:
        case GFS_REMOVE_AMMO:
        case GFS_REMOVE_MEDIPACKS:
        case GFS_SETUP_BACON_LARA:
        case GFS_LEGACY:
            break;
        }
    }
    Memory_Free(sequence->events);
}

void GF_Shutdown(void)
{
    Memory_FreePointer(&g_GameFlow.main_menu_background_path);
    Memory_FreePointer(&g_GameFlow.savegame_fmt_legacy);
    Memory_FreePointer(&g_GameFlow.savegame_fmt_bson);
    Memory_FreePointer(&g_GameInfo.current);

    for (int i = 0; i < g_GameFlow.injections.length; i++) {
        Memory_FreePointer(&g_GameFlow.injections.data_paths[i]);
    }
    Memory_FreePointer(&g_GameFlow.injections.data_paths);

    if (g_GameFlow.levels != NULL) {
        for (int i = 0; i < g_GameFlow.level_count; i++) {
            GAME_FLOW_LEVEL *const level = &g_GameFlow.levels[i];
            Memory_FreePointer(&level->path);
            Memory_FreePointer(&level->title);

            for (int j = 0; j < level->injections.length; j++) {
                Memory_FreePointer(&level->injections.data_paths[j]);
            }
            Memory_FreePointer(&level->injections.data_paths);

            if (level->item_drops.count) {
                for (int j = 0; j < level->item_drops.count; j++) {
                    Memory_FreePointer(&level->item_drops.data[j].object_ids);
                }
                Memory_FreePointer(&level->item_drops.data);
            }

            M_FreeSequence(&level->sequence);
        }
        Memory_FreePointer(&g_GameFlow.levels);
    }
}

int32_t GF_GetLevelCount(void)
{
    return g_GameFlow.level_count;
}

int32_t GF_GetDemoCount(void)
{
    int32_t demo_count = 0;
    for (int32_t i = g_GameFlow.first_level_num; i <= g_GameFlow.last_level_num;
         i++) {
        if (g_GameFlow.levels[i].demo) {
            demo_count++;
        }
    }
    return demo_count;
}

const char *GF_GetLevelPath(const int32_t level_num)
{
    return g_GameFlow.levels[level_num].path;
}

const char *GF_GetLevelTitle(const int32_t level_num)
{
    return g_GameFlow.levels[level_num].title;
}

void GF_SetLevelTitle(const int32_t level_num, const char *const title)
{
    Memory_FreePointer(&g_GameFlow.levels[level_num].title);
    g_GameFlow.levels[level_num].title =
        title != NULL ? Memory_DupStr(title) : NULL;
}

int32_t GF_GetGymLevelNum(void)
{
    return g_GameFlow.gym_level_num;
}
