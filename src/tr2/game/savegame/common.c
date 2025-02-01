#include "decomp/savegame.h"
#include "game/game_flow.h"
#include "global/vars.h"

#include <libtrx/debug.h>
#include <libtrx/enum_map.h>
#include <libtrx/game/savegame.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

void Savegame_Init(void)
{
    g_SaveGame.start = Memory_Alloc(
        sizeof(START_INFO)
        * (GF_GetLevelTable(GFLT_MAIN)->count
           + GF_GetLevelTable(GFLT_DEMOS)->count));

    const GF_LEVEL_TABLE *const level_table = GF_GetLevelTable(GFLT_DEMOS);
    for (int32_t i = 0; i < level_table->count; i++) {
        START_INFO *const resume_info =
            Savegame_GetCurrentInfo(&level_table->levels[i]);
        resume_info->available = 1;
        resume_info->has_pistols = 1;
        resume_info->pistol_ammo = 1000;
        resume_info->gun_status = LGS_ARMLESS;
        resume_info->gun_type = LGT_PISTOLS;
    }
}

void Savegame_Shutdown(void)
{
    Memory_FreePointer(&g_SaveGame.start);
}

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
    ASSERT(g_SaveGame.start != nullptr);
    ASSERT(level != nullptr);
    if (GF_GetLevelTableType(level->type) == GFLT_MAIN) {
        return &g_SaveGame.start[level->num];
    } else if (level->type == GFL_DEMO) {
        return &g_SaveGame.start[GF_GetLevelTable(GFLT_MAIN)->count];
    }
    LOG_WARNING(
        "Warning: unable to get resume info for level %d (type=%s)", level->num,
        ENUM_MAP_TO_STRING(GF_LEVEL_TYPE, level->type));
    return nullptr;
}
