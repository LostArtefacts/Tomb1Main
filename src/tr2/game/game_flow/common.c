#include "game/game_flow/common.h"

#include "game/game_flow/vars.h"
#include "global/vars.h"

#include <libtrx/memory.h>

int32_t GF_GetLevelCount(void)
{
    return g_GameFlow.level_count;
}

int32_t GF_GetDemoCount(void)
{
    return g_GameFlow.demo_level_count;
}

const char *GF_GetLevelFileName(int32_t level_num)
{
    return g_GF_LevelFileNames[level_num];
}

const char *GF_GetLevelTitle(const int32_t level_num)
{
    return g_GF_LevelNames[level_num];
}

void GF_SetLevelTitle(const int32_t level_num, const char *const title)
{
    Memory_FreePointer(&g_GF_LevelNames[level_num]);
    g_GF_LevelNames[level_num] = title != NULL ? Memory_DupStr(title) : NULL;
}

int32_t GF_GetGymLevelNum(void)
{
    return g_GameFlow.gym_enabled ? LV_GYM : -1;
}

void GF_OverrideCommand(const GAME_FLOW_COMMAND command)
{
    g_GF_OverrideCommand = command;
}

GAME_FLOW_COMMAND GF_GetOverrideCommand(void)
{
    return g_GF_OverrideCommand;
}
