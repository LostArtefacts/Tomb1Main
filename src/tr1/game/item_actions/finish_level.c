#include "game/item_actions/finish_level.h"

#include "global/vars.h"

#include <stdbool.h>

void ItemAction_FinishLevel(ITEM *item)
{
    g_LevelComplete = true;
}
