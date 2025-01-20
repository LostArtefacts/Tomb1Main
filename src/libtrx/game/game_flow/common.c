#include "game/game_flow/common.h"

bool GF_IsGymEnabled(void)
{
    return GF_GetGymLevelNum() != -1;
}
