#include "game/game_flow/common.h"

#include "game/game_flow/vars.h"
#include "global/vars.h"

#include <libtrx/log.h>
#include <libtrx/memory.h>

static void M_FreeSequence(GAME_FLOW_SEQUENCE *sequence);
static void M_FreeLevel(GAME_FLOW_LEVEL *level);
static void M_FreeLevels(GAME_FLOW_LEVEL **levels, int32_t *level_count);
static void M_FreeDemos(GAME_FLOW *gf);
static void M_FreeCutscenes(GAME_FLOW *gf);
static void M_FreeFMVs(GAME_FLOW *gf);

static void M_FreeSequence(GAME_FLOW_SEQUENCE *const sequence)
{
    Memory_Free(sequence->events);
}

static void M_FreeLevel(GAME_FLOW_LEVEL *const level)
{
    for (int32_t j = 0; j < level->injections.count; j++) {
        Memory_FreePointer(&level->injections.data_paths[j]);
    }
    M_FreeSequence(&level->sequence);
    Memory_FreePointer(&level->injections.data_paths);
    Memory_FreePointer(&level->path);
    Memory_FreePointer(&level->title);
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

int32_t GF_GetLevelCount(void)
{
    return g_GameFlow.level_count;
}

int32_t GF_GetDemoCount(void)
{
    return g_GameFlow.demo_count;
}

const char *GF_GetLevelTitle(const int32_t level_num)
{
    return g_GameFlow.levels[level_num].title;
}

int32_t GF_GetCutsceneCount(void)
{
    return g_GameFlow.cutscene_count;
}

void GF_SetLevelTitle(const int32_t level_num, const char *const title)
{
    Memory_FreePointer(&g_GameFlow.levels[level_num].title);
    g_GameFlow.levels[level_num].title =
        title != NULL ? Memory_DupStr(title) : NULL;
}

int32_t GF_GetGymLevelNum(void)
{
    return g_GameFlow.gym_enabled ? LV_GYM : -1;
}

void GF_Shutdown(void)
{
    GAME_FLOW *const gf = &g_GameFlow;

    for (int32_t i = 0; i < gf->injections.count; i++) {
        Memory_FreePointer(&gf->injections.data_paths[i]);
    }
    Memory_FreePointer(&gf->injections.data_paths);

    M_FreeLevels(&gf->levels, &gf->level_count);
    M_FreeLevels(&gf->demos, &gf->demo_count);
    M_FreeLevels(&gf->cutscenes, &gf->cutscene_count);
    M_FreeFMVs(gf);

    if (gf->title_level != NULL) {
        M_FreeLevel(gf->title_level);
        Memory_FreePointer(&gf->title_level);
    }
}

GAME_FLOW_LEVEL *GF_GetLevel(
    const int32_t num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    switch (level_type) {
    case GFL_TITLE:
        return g_GameFlow.title_level;

    case GFL_CUTSCENE:
        if (num < 0 || num >= GF_GetCutsceneCount()) {
            LOG_ERROR("Invalid cutscene number: %d", num);
            return NULL;
        }
        return &g_GameFlow.cutscenes[num];

    case GFL_DEMO:
        if (num < 0 || num >= GF_GetDemoCount()) {
            LOG_ERROR("Invalid demo number: %d", num);
            return NULL;
        }
        return &g_GameFlow.demos[num];

    case GFL_NORMAL:
    case GFL_SAVED:
        if (num < 0 || num >= GF_GetLevelCount()) {
            LOG_ERROR("Invalid level number: %d", num);
            return NULL;
        }
        return &g_GameFlow.levels[num];

    default:
        LOG_ERROR("Invalid level type: %d", level_type);
        return NULL;
    }
}

START_INFO *GF_GetResumeInfo(const GAME_FLOW_LEVEL *const level)
{
    if (level->type == GFL_NORMAL) {
        return &g_SaveGame.start[level->num];
    } else if (level->type == GFL_DEMO) {
        return &g_SaveGame.start[0];
    }
    return NULL;
}
