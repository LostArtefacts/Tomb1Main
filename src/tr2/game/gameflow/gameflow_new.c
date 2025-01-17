#include "game/gameflow/gameflow_new.h"

#include "global/vars.h"

#include <libtrx/memory.h>

GAME_FLOW_NEW g_GameFlowNew;
GAME_INFO g_GameInfo;

int32_t GameFlow_GetLevelCount(void)
{
    return g_GameFlowNew.level_count;
}

int32_t GameFlow_GetDemoCount(void)
{
    return g_GameFlow.num_demos;
}

const char *GameFlow_GetLevelFileName(int32_t level_num)
{
    return g_GF_LevelFileNames[level_num];
}

const char *GameFlow_GetLevelTitle(const int32_t level_num)
{
    return g_GF_LevelNames[level_num];
}

void GameFlow_SetLevelTitle(const int32_t level_num, const char *const title)
{
    Memory_FreePointer(&g_GF_LevelNames[level_num]);
    g_GF_LevelNames[level_num] = title != NULL ? Memory_DupStr(title) : NULL;
}

int32_t GameFlow_GetGymLevelNum(void)
{
    return g_GameFlow.gym_enabled ? LV_GYM : -1;
}

void GameFlow_OverrideCommand(const GAME_FLOW_COMMAND command)
{
    g_GF_OverrideCommand = command;
}

GAME_FLOW_COMMAND GameFlow_GetOverrideCommand(void)
{
    return g_GF_OverrideCommand;
}
