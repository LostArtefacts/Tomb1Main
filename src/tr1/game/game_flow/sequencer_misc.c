#include "game/demo.h"
#include "game/game_flow/common.h"
#include "game/game_flow/sequencer.h"
#include "game/game_flow/vars.h"
#include "game/savegame.h"

#include <libtrx/log.h>

GAME_FLOW_COMMAND GF_PlayAvailableStory(const int32_t slot_num)
{
    const int32_t savegame_level = Savegame_GetLevelNumber(slot_num);
    for (int32_t i = g_GameFlow.first_level_num; i <= savegame_level; i++) {
        const GAME_FLOW_COMMAND gf_cmd = GF_InterpretSequence(
            GF_GetLevel(i, GFL_NORMAL), GFSC_STORY,
            (void *)(intptr_t)savegame_level);
        if (gf_cmd.action == GF_EXIT_TO_TITLE
            || gf_cmd.action == GF_EXIT_GAME) {
            break;
        }
    }
    return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
}

GAME_FLOW_COMMAND GF_DoCutsceneSequence(const int32_t cutscene_num)
{
    const GAME_FLOW_LEVEL *const level =
        GF_GetLevel(cutscene_num, GFL_CUTSCENE);
    if (level == NULL) {
        LOG_ERROR("Missing cutscene: %d", cutscene_num);
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(
        GF_GetLevel(cutscene_num, GFL_CUTSCENE), GFSC_NORMAL, NULL);
}

GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num)
{
    demo_num = Demo_ChooseLevel(demo_num);
    if (demo_num < 0) {
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(
        GF_GetLevel(demo_num, GFL_DEMO), GFSC_NORMAL, NULL);
}
