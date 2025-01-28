#include "game/demo.h"
#include "game/game.h"
#include "game/game_flow/common.h"
#include "game/game_flow/sequencer.h"
#include "game/game_flow/vars.h"

#include <libtrx/log.h>

#include <stddef.h>
GAME_FLOW_COMMAND GF_DoDemoSequence(int32_t demo_num)
{
    demo_num = Demo_ChooseLevel(demo_num);
    if (demo_num < 0) {
        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
    }
    const GAME_FLOW_LEVEL *const level = GF_GetLevel(demo_num, GFL_DEMO);
    if (level == NULL) {
        LOG_ERROR("Missing demo: %d", demo_num);
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(level, GFSC_NORMAL, NULL);
}

GAME_FLOW_COMMAND GF_DoCutsceneSequence(const int32_t cutscene_num)
{
    const GAME_FLOW_LEVEL *const level =
        GF_GetLevel(cutscene_num, GFL_CUTSCENE);
    if (level == NULL) {
        LOG_ERROR("Missing cutscene: %d", cutscene_num);
        return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(level, GFSC_NORMAL, NULL);
}

bool GF_DoFrontendSequence(void)
{
    if (g_GameFlow.title_level == NULL) {
        return false;
    }
    const GAME_FLOW_COMMAND gf_cmd =
        GF_InterpretSequence(g_GameFlow.title_level, GFSC_NORMAL, NULL);
    return gf_cmd.action == GF_EXIT_GAME;
}

GAME_FLOW_COMMAND GF_DoLevelSequence(
    const GAME_FLOW_LEVEL *const start_level,
    const GAME_FLOW_SEQUENCE_CONTEXT seq_ctx)
{
    const GAME_FLOW_LEVEL *current_level = start_level;
    const int32_t level_count = GF_GetLevelCount(current_level->type);
    while (true) {
        const GAME_FLOW_COMMAND gf_cmd =
            GF_InterpretSequence(current_level, seq_ctx, NULL);

        if (g_GameFlow.single_level >= 0) {
            return gf_cmd;
        }
        if (gf_cmd.action != GF_NOOP && gf_cmd.action != GF_LEVEL_COMPLETE) {
            return gf_cmd;
        }
        if (Game_IsInGym()) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        if (current_level->num >= level_count - 1) {
            return (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
        }
        current_level++;
    }
}
