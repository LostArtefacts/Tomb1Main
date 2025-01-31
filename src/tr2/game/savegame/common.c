#include "decomp/savegame.h"
#include "game/game_flow.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/enum_map.h>
#include <libtrx/game/savegame.h>
#include <libtrx/log.h>

int32_t Savegame_GetSlotCount(void)
{
    return MAX_SAVE_SLOTS;
}

bool Savegame_IsSlotFree(const int32_t slot_idx)
{
    return g_SavedLevels[slot_idx] == 0;
}

bool Savegame_Save(const int32_t slot_idx)
{
    CreateSaveGameInfo();
    S_SaveGame(slot_idx);
    GetSavedGamesList(&g_LoadGameRequester);
    return true;
}

START_INFO *Savegame_GetCurrentInfo(const GF_LEVEL *const level)
{
    ASSERT(level != nullptr);
    if (GF_GetLevelTableType(level->type) == GFLT_MAIN) {
        return &g_SaveGame.start[level->num];
    } else if (level->type == GFL_DEMO) {
        return &g_SaveGame.start[0];
    }
    LOG_WARNING(
        "Warning: unable to get resume info for level %d (type=%s)", level->num,
        ENUM_MAP_TO_STRING(GF_LEVEL_TYPE, level->type));
    return nullptr;
}
