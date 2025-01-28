#include "game/game_flow/common.h"

#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/enum_map.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

START_INFO *GF_GetResumeInfo(const GF_LEVEL *const level)
{
    ASSERT(level != NULL);
    if (GF_GetLevelTableType(level->type) == GFLT_MAIN) {
        return &g_SaveGame.start[level->num];
    } else if (level->type == GFL_DEMO) {
        return &g_SaveGame.start[0];
    }
    LOG_WARNING(
        "Warning: unable to get resume info for level %d (type=%s)", level->num,
        ENUM_MAP_TO_STRING(GF_LEVEL_TYPE, level->type));
    return NULL;
}
