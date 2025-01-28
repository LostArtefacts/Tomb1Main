#include "game/demo.h"
#include "game/game_flow/common.h"
#include "game/game_flow/sequencer.h"
#include "game/game_flow/vars.h"
#include "game/savegame.h"

#include <libtrx/log.h>

GF_COMMAND GF_PlayAvailableStory(const int32_t slot_num)
{
    const int32_t savegame_level = Savegame_GetLevelNumber(slot_num);
    const GF_LEVEL_TABLE *const level_table = GF_GetLevelTable(GFLT_MAIN);
    for (int32_t i = 0; i <= MIN(savegame_level, level_table->count); i++) {
        const GF_LEVEL *const level = GF_GetLevel(GFLT_MAIN, i);
        if (level->type == GFL_GYM) {
            continue;
        }
        const GF_COMMAND gf_cmd = GF_InterpretSequence(
            level, GFSC_STORY, (void *)(intptr_t)savegame_level);
        if (gf_cmd.action == GF_EXIT_TO_TITLE
            || gf_cmd.action == GF_EXIT_GAME) {
            break;
        }
    }
    return (GF_COMMAND) { .action = GF_EXIT_TO_TITLE };
}

GF_COMMAND GF_DoCutsceneSequence(const int32_t cutscene_num)
{
    const GF_LEVEL *const level = GF_GetLevel(GFLT_CUTSCENES, cutscene_num);
    if (level == NULL) {
        LOG_ERROR("Missing cutscene: %d", cutscene_num);
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(level, GFSC_NORMAL, NULL);
}

GF_COMMAND GF_DoDemoSequence(int32_t demo_num)
{
    demo_num = Demo_ChooseLevel(demo_num);
    if (demo_num < 0) {
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    const GF_LEVEL *const level = GF_GetLevel(GFLT_DEMOS, demo_num);
    if (level == NULL) {
        LOG_ERROR("Missing cutscene: %d", demo_num);
        return (GF_COMMAND) { .action = GF_NOOP };
    }
    return GF_InterpretSequence(level, GFSC_NORMAL, NULL);
}
