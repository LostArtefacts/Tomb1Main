#include "game/game_flow/common.h"

static GAME_FLOW_COMMAND m_OverrideCommand = { .action = GF_NOOP };

bool GF_IsGymEnabled(void)
{
    return GF_GetGymLevelNum() != -1;
}

void GF_OverrideCommand(const GAME_FLOW_COMMAND command)
{
    m_OverrideCommand = command;
}

GAME_FLOW_COMMAND GF_GetOverrideCommand(void)
{
    return m_OverrideCommand;
}
