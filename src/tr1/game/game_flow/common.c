#include "game/game_flow/common.h"

#include "game/game_flow/vars.h"
#include "global/vars.h"

#include <libtrx/log.h>
#include <libtrx/memory.h>

static void M_FreeSequence(GAME_FLOW_SEQUENCE *sequence);
static void M_FreeInjections(INJECTION_DATA *injections);
static void M_FreeLevel(GAME_FLOW_LEVEL *level);
static void M_FreeLevels(GAME_FLOW_LEVEL **levels, int32_t *level_count);
static void M_FreeFMVs(GAME_FLOW *gf);

static void M_FreeSequence(GAME_FLOW_SEQUENCE *const sequence)
{
    Memory_Free(sequence->events);
}

static void M_FreeInjections(INJECTION_DATA *const injections)
{
    for (int32_t i = 0; i < injections->count; i++) {
        Memory_FreePointer(&injections->data_paths[i]);
    }
    Memory_FreePointer(&injections->data_paths);
}

static void M_FreeLevel(GAME_FLOW_LEVEL *const level)
{
    Memory_FreePointer(&level->path);
    Memory_FreePointer(&level->title);
    M_FreeInjections(&level->injections);
    M_FreeSequence(&level->sequence);

    if (level->item_drops.count > 0) {
        for (int32_t i = 0; i < level->item_drops.count; i++) {
            Memory_FreePointer(&level->item_drops.data[i].object_ids);
        }
        Memory_FreePointer(&level->item_drops.data);
    }
}

static void M_FreeLevels(GAME_FLOW_LEVEL **levels, int32_t *const level_count)
{
    if (levels != NULL) {
        for (int32_t i = 0; i < *level_count; i++) {
            M_FreeLevel(&(*levels)[i]);
        }
        Memory_FreePointer(levels);
    }
    *level_count = 0;
}

static void M_FreeFMVs(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->fmv_count; i++) {
        Memory_FreePointer(&gf->fmvs[i].path);
    }
    Memory_FreePointer(&gf->fmvs);
    gf->fmv_count = 0;
}

void GF_Shutdown(void)
{
    GAME_FLOW *const gf = &g_GameFlow;

    Memory_FreePointer(&gf->main_menu_background_path);
    Memory_FreePointer(&gf->savegame_fmt_legacy);
    Memory_FreePointer(&gf->savegame_fmt_bson);
    Memory_FreePointer(&g_GameInfo.current);

    M_FreeInjections(&gf->injections);
    M_FreeLevels(&gf->levels, &gf->level_count);
    M_FreeLevels(&gf->demos, &gf->demo_count);
    M_FreeLevels(&gf->cutscenes, &gf->cutscene_count);
    M_FreeFMVs(gf);
}

int32_t GF_GetLevelCount(const GAME_FLOW_LEVEL_TYPE level_type)
{
    switch (level_type) {
    case GFL_TITLE:
        return 1;
    case GFL_GYM:
        return 1;
    case GFL_CUTSCENE:
        return g_GameFlow.cutscene_count;
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

GAME_FLOW_LEVEL *GF_GetLevel(
    const int32_t num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    switch (level_type) {
    case GFL_TITLE:
        return &g_GameFlow.levels[g_GameFlow.title_level_num];

    case GFL_CUTSCENE:
        if (num < 0 || num >= GF_GetLevelCount(level_type)) {
            LOG_ERROR("Invalid cutscene number: %d", num);
            return NULL;
        }
        return &g_GameFlow.cutscenes[num];

    case GFL_DEMO:
        if (num < 0 || num >= GF_GetLevelCount(level_type)) {
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
