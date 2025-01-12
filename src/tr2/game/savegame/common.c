#include "decomp/savegame.h"
#include "global/vars.h"

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
