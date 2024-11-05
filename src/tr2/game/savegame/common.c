#include "global/funcs.h"
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
    const int16_t slot_num = g_Inv_ExtraData[1];
    S_SaveGame(&g_SaveGame, sizeof(SAVEGAME_INFO), slot_idx);
    // TODO: move me inside S_SaveGame
    g_SaveGameReqFlags1[slot_num] = g_RequesterFlags1[slot_num];
    g_SaveGameReqFlags2[slot_num] = g_RequesterFlags2[slot_num];
    GetSavedGamesList(&g_LoadGameRequester);
    return true;
}
