#include "game/item_actions/finish_level.h"

#include "global/vars.h"

void ItemAction_FinishLevel(ITEM *item)
{
    g_LevelComplete = true;
}
