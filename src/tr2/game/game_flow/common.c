#include "game/game_flow/common.h"

#include "game/game_flow/vars.h"
#include "global/vars.h"

#include <libtrx/memory.h>

static void M_FreeSequence(GAME_FLOW_SEQUENCE *sequence);
static void M_FreeLevel(GAME_FLOW_LEVEL *level);
static void M_FreeLevels(GAME_FLOW *gf);
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

static void M_FreeLevels(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->level_count; i++) {
        M_FreeLevel(&gf->levels[i]);
    }
    Memory_FreePointer(&gf->levels);
    gf->level_count = 0;
}

static void M_FreeDemos(GAME_FLOW *const gf)
{
    Memory_FreePointer(&gf->demos);
    gf->demo_count = 0;
}

static void M_FreeCutscenes(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->cutscene_count; i++) {
        Memory_FreePointer(&gf->cutscenes[i].path);
    }
    Memory_FreePointer(&gf->cutscenes);
    gf->cutscene_count = 0;
}

static void M_FreeFMVs(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->fmv_count; i++) {
        Memory_FreePointer(&gf->fmvs[i].path);
    }
    Memory_FreePointer(&gf->fmvs);
    gf->fmv_count = 0;
}

const char *GF_GetTitleLevelPath(void)
{
    if (g_GameFlow.title_level == NULL) {
        return NULL;
    }
    return g_GameFlow.title_level->path;
}

int32_t GF_GetLevelCount(void)
{
    return g_GameFlow.level_count;
}

int32_t GF_GetDemoCount(void)
{
    return g_GameFlow.demo_count;
}

const char *GF_GetLevelPath(const int32_t level_num)
{
    return g_GameFlow.levels[level_num].path;
}

const char *GF_GetLevelTitle(const int32_t level_num)
{
    return g_GameFlow.levels[level_num].title;
}

int32_t GF_GetCutsceneCount(void)
{
    return g_GameFlow.cutscene_count;
}

const char *GF_GetCutscenePath(const int32_t cutscene_num)
{
    return g_GameFlow.cutscenes[cutscene_num].path;
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

    M_FreeLevels(gf);
    M_FreeDemos(gf);
    M_FreeCutscenes(gf);
    M_FreeFMVs(gf);

    if (gf->title_level != NULL) {
        M_FreeLevel(gf->title_level);
        Memory_FreePointer(&gf->title_level);
    }
}
