#include "config.h"
#include "game/demo.h"
#include "game/game_flow/common.h"
#include "game/game_flow/sequencer.h"
#include "game/game_flow/vars.h"
#include "game/inventory_ring/control.h"
#include "game/objects/vars.h"
#include "game/phase.h"
#include "log.h"

GF_COMMAND GF_EnterPhotoMode(void)
{
    PHASE *const subphase = Phase_PhotoMode_Create();
    const GF_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
    Phase_PhotoMode_Destroy(subphase);
    return gf_cmd;
}

GF_COMMAND GF_PauseGame(void)
{
    PHASE *const subphase = Phase_Pause_Create();
    const GF_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
    Phase_Pause_Destroy(subphase);
    return gf_cmd;
}

GF_COMMAND GF_ShowInventory(const INVENTORY_MODE mode)
{
    PHASE *const phase = Phase_Inventory_Create(mode);
    const GF_COMMAND gf_cmd = PhaseExecutor_Run(phase);
    Phase_Inventory_Destroy(phase);
    return gf_cmd;
}

GF_COMMAND GF_ShowInventoryKeys(const GAME_OBJECT_ID receptacle_type_id)
{
    if (g_Config.gameplay.enable_auto_item_selection) {
        const GAME_OBJECT_ID object_id = Object_GetCognateInverse(
            receptacle_type_id, g_KeyItemToReceptacleMap);
        InvRing_SetRequestedObjectID(object_id);
    }
    return GF_ShowInventory(INV_KEYS_MODE);
}

GF_COMMAND GF_RunDemo(const int32_t demo_num)
{
    PHASE *const demo_phase = Phase_Demo_Create(demo_num);
    const GF_COMMAND gf_cmd = PhaseExecutor_Run(demo_phase);
    Phase_Demo_Destroy(demo_phase);
    return gf_cmd;
}

GF_COMMAND GF_RunCutscene(const int32_t cutscene_num)
{
    PHASE *const cutscene_phase = Phase_Cutscene_Create(cutscene_num);
    const GF_COMMAND gf_cmd = PhaseExecutor_Run(cutscene_phase);
    Phase_Cutscene_Destroy(cutscene_phase);
    return gf_cmd;
}

GF_COMMAND GF_RunGame(
    const GF_LEVEL *const level, const GF_SEQUENCE_CONTEXT seq_ctx)
{
    PHASE *const phase = Phase_Game_Create(level, seq_ctx);
    const GF_COMMAND gf_cmd = PhaseExecutor_Run(phase);
    Phase_Game_Destroy(phase);
    return gf_cmd;
}

GF_COMMAND GF_DoFrontendSequence(void)
{
    if (g_GameFlow.title_level == nullptr) {
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(g_GameFlow.title_level, GFSC_NORMAL, nullptr);
}

GF_COMMAND GF_DoDemoSequence(int32_t demo_num)
{
    demo_num = Demo_ChooseLevel(demo_num);
    if (demo_num < 0) {
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    const GF_LEVEL *const level = GF_GetLevel(GFLT_DEMOS, demo_num);
    if (level == nullptr) {
        LOG_ERROR("Missing demo: %d", demo_num);
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(level, GFSC_NORMAL, nullptr);
}

GF_COMMAND GF_DoCutsceneSequence(const int32_t cutscene_num)
{
    const GF_LEVEL *const level = GF_GetLevel(GFLT_CUTSCENES, cutscene_num);
    if (level == nullptr) {
        LOG_ERROR("Missing cutscene: %d", cutscene_num);
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(level, GFSC_NORMAL, nullptr);
}
