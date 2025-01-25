#include "game/game_flow/common.h"

#include "game/game_flow/vars.h"
#include "global/vars.h"

#include <libtrx/log.h>
#include <libtrx/memory.h>

static void M_FreeSequence(GAME_FLOW_SEQUENCE *sequence);

static void M_FreeSequence(GAME_FLOW_SEQUENCE *const sequence)
{
    Memory_Free(sequence->events);
}

void GF_Shutdown(void)
{
    Memory_FreePointer(&g_GameFlow.main_menu_background_path);
    Memory_FreePointer(&g_GameFlow.savegame_fmt_legacy);
    Memory_FreePointer(&g_GameFlow.savegame_fmt_bson);
    Memory_FreePointer(&g_GameInfo.current);

    for (int i = 0; i < g_GameFlow.injections.count; i++) {
        Memory_FreePointer(&g_GameFlow.injections.data_paths[i]);
    }
    Memory_FreePointer(&g_GameFlow.injections.data_paths);

    if (g_GameFlow.levels != NULL) {
        for (int i = 0; i < g_GameFlow.level_count; i++) {
            GAME_FLOW_LEVEL *const level = &g_GameFlow.levels[i];
            Memory_FreePointer(&level->path);
            Memory_FreePointer(&level->title);

            for (int j = 0; j < level->injections.count; j++) {
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

int32_t GF_GetLevelCount(const GAME_FLOW_LEVEL_TYPE level_type)
{
    switch (level_type) {
    case GFL_TITLE:
        return 1;
    case GFL_GYM:
        return 1;
    case GFL_DEMO:
        return g_GameFlow.demo_count;
    default:
        return g_GameFlow.level_count;
    }
}

int32_t GF_GetDemoCount(void)
{
    return g_GameFlow.demo_count;
}

void GF_SetLevelTitle(GAME_FLOW_LEVEL *const level, const char *const title)
{
    Memory_FreePointer(&level->title);
    level->title = title != NULL ? Memory_DupStr(title) : NULL;
}

int32_t GF_GetGymLevelNum(void)
{
    return g_GameFlow.gym_level_num;
}

GAME_FLOW_LEVEL *GF_GetCurrentLevel(void)
{
    return GF_GetLevel(g_CurrentLevel, g_GameInfo.current_level_type);
}

GAME_FLOW_LEVEL *GF_GetLevel(
    const int32_t num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    switch (level_type) {
    case GFL_TITLE:
        return &g_GameFlow.levels[g_GameFlow.title_level_num];

    case GFL_DEMO:
        if (num < 0 || num >= GF_GetDemoCount()) {
            LOG_ERROR("Invalid demo number: %d", num);
            return NULL;
        }
        return &g_GameFlow.demos[num];

    default:
        if (num < 0 || num >= GF_GetLevelCount(level_type)) {
            LOG_ERROR("Invalid level number: %d", num);
            return NULL;
        }
        return &g_GameFlow.levels[num];
    }
}

RESUME_INFO *GF_GetResumeInfo(const GAME_FLOW_LEVEL *const level)
{
    if (level->type == GFL_NORMAL) {
        return &g_GameInfo.current[level->num];
    } else if (level->type == GFL_GYM) {
        return &g_GameInfo.current[g_GameFlow.gym_level_num];
    } else if (level->type == GFL_DEMO) {
        return &g_GameInfo.current[0];
    }
    return NULL;
}
