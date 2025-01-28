#include "game/game_flow/common.h"

#include "game/game_flow/vars.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

int32_t GF_GetLevelCount(const GAME_FLOW_LEVEL_TYPE level_type)
{
    switch (level_type) {
    case GFL_NORMAL:
        return g_GameFlow.level_count;
    case GFL_CUTSCENE:
        return g_GameFlow.cutscene_count;
    case GFL_DEMO:
        return g_GameFlow.demo_count;
    default:
        ASSERT_FAIL();
    }
}

int32_t GF_GetDemoCount(void)
{
    return g_GameFlow.demo_count;
}

int32_t GF_GetCutsceneCount(void)
{
    return g_GameFlow.cutscene_count;
}

void GF_SetLevelTitle(GAME_FLOW_LEVEL *const level, const char *const title)
{
    Memory_FreePointer(&level->title);
    level->title = title != NULL ? Memory_DupStr(title) : NULL;
}

int32_t GF_GetGymLevelNum(void)
{
    return g_GameFlow.gym_enabled ? LV_GYM : -1;
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
        if (num < 0 || num >= GF_GetLevelCount(GFL_NORMAL)) {
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
