#include "game/game_flow/sequencer.h"

#include "config.h"
#include "game/inventory_ring/control.h"
#include "game/objects/vars.h"
#include "game/phase.h"

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

GAME_FLOW_COMMAND GF_RunDemo(const int32_t demo_num)
{
    PHASE *const demo_phase = Phase_Demo_Create(demo_num);
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(demo_phase);
    Phase_Demo_Destroy(demo_phase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_RunCutscene(const int32_t cutscene_num)
{
    PHASE *const cutscene_phase = Phase_Cutscene_Create(cutscene_num);
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(cutscene_phase);
    Phase_Cutscene_Destroy(cutscene_phase);
    return gf_cmd;
}

GAME_FLOW_COMMAND GF_RunGame(
    const int32_t level_num, const GAME_FLOW_LEVEL_TYPE level_type)
{
    PHASE *const phase = Phase_Game_Create(level_num, level_type);
    const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(phase);
    Phase_Game_Destroy(phase);
    return gf_cmd;
}
