#include "game/game_flow/sequencer.h"

#include "game/demo.h"
#include "game/game_flow.h"
#include "game/inventory_ring.h"
#include "game/phase.h"

#include <libtrx/config.h>
#include <libtrx/game/objects/vars.h>

GAME_FLOW_COMMAND GF_StartDemo(const int32_t demo_num)
{
    const int32_t level_num = Demo_ChooseLevel(demo_num);
    if (level_num < 0) {
        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
    return GF_DoLevelSequence(level_num, GFL_DEMO);
}

GAME_FLOW_COMMAND GF_StartGame(
    const int32_t level_num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    if (level_type == GFL_DEMO) {
        PHASE *const demo_phase = Phase_Demo_Create(level_num);
        const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(demo_phase);
        Phase_Demo_Destroy(demo_phase);
        return gf_cmd;
    } else {
        PHASE *const phase = Phase_Game_Create(level_num, level_type);
        const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
        Phase_Game_Destroy(phase);
        return gf_cmd;
    }
}

GAME_FLOW_COMMAND GF_EnterPhotoMode(void)
{
    PHASE *const subphase = Phase_PhotoMode_Create();
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
    Phase_PhotoMode_Destroy(subphase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_PauseGame(void)
{
    PHASE *const subphase = Phase_Pause_Create();
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
    Phase_Pause_Destroy(subphase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_ShowInventory(const INVENTORY_MODE mode)
{
    PHASE *const phase = Phase_Inventory_Create(mode);
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
    Phase_Inventory_Destroy(phase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_ShowInventoryKeys(const GAME_OBJECT_ID receptacle_type_id)
{
    if (g_Config.gameplay.enable_auto_item_selection) {
        const GAME_OBJECT_ID object_id = Object_GetCognateInverse(
            receptacle_type_id, g_KeyItemToReceptacleMap);
        InvRing_SetRequestedObjectID(object_id);
    }
    return GF_ShowInventory(INV_KEYS_MODE);
}
