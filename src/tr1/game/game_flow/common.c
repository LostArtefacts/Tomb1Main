#include "game/game_flow/common.h"

#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/enum_map.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

RESUME_INFO *GF_GetResumeInfo(const GAME_FLOW_LEVEL *const level)
{
    ASSERT(level != NULL);
    if (GF_GetLevelTableType(level->type) == GFLT_MAIN) {
        return &g_GameInfo.current[level->num];
    } else if (level->type == GFL_DEMO) {
        return &g_GameInfo.current[0];
    }
    LOG_WARNING(
        "Warning: unable to get resume info for level %d (type=%s)", level->num,
        ENUM_MAP_TO_STRING(GAME_FLOW_LEVEL_TYPE, level->type));
    return NULL;
}
