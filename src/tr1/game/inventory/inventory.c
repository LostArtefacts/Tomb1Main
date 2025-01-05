#include "game/inventory/inventory_ring.h"
#include "game/inventory/inventory_vars.h"
#include "game/phase/phase.h"
#include "game/phase/phase_inventory.h"
#include "global/types.h"

#include <libtrx/config.h>
#include <libtrx/game/objects/vars.h>
#include <libtrx/memory.h>

bool Inv_Display(const INV_MODE inv_mode)
{
    if (inv_mode == INV_KEYS_MODE && !g_InvKeysObjects) {
        return false;
    }
    PHASE_INVENTORY_ARGS *const args =
        Memory_Alloc(sizeof(PHASE_INVENTORY_ARGS));
    args->mode = inv_mode;
    Phase_Set(PHASE_INVENTORY, args);
    return true;
}

bool Inv_DisplayKeys(const GAME_OBJECT_ID receptacle_type_id)
{
    if (g_Config.gameplay.enable_auto_item_selection) {
        const GAME_OBJECT_ID object_id = Object_GetCognateInverse(
            receptacle_type_id, g_KeyItemToReceptacleMap);
        InvRing_SetRequestedObjectID(object_id);
    }

    return Inv_Display(INV_KEYS_MODE);
}
