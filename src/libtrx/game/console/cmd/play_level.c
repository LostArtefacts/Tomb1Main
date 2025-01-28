#include "game/console/cmd/play_level.h"

#include "game/game_flow/common.h"
#include "game/game_string.h"
#include "strings.h"

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *ctx);

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *const ctx)
{
    if (String_IsEmpty(ctx->args)) {
        return CR_BAD_INVOCATION;
    }

    VECTOR *source = NULL;
    VECTOR *matches = NULL;
    int32_t level_to_load = -1;
    const GF_LEVEL_TABLE *const level_table = GF_GetLevelTable(GFLT_MAIN);

    if (String_ParseInteger(ctx->args, &level_to_load)) {
        goto matched;
    }

    source = Vector_Create(sizeof(STRING_FUZZY_SOURCE));
    for (int32_t i = 0; i < level_table->count; i++) {
        STRING_FUZZY_SOURCE source_item = {
            .key = level_table->levels[i].title,
            .value = (void *)(intptr_t)i,
            .weight = 1,
        };
        Vector_Add(source, &source_item);
    }

    const GF_LEVEL *const gym_level = GF_GetGymLevel();
    if (gym_level != NULL) {
        STRING_FUZZY_SOURCE source_item = {
            .key = "gym",
            .value = (void *)(intptr_t)gym_level->num,
            .weight = 1,
        };
        Vector_Add(source, &source_item);
    }

    COMMAND_RESULT result;
    matches = String_FuzzyMatch(ctx->args, source);

    if (matches->count == 0) {
        Console_Log(GS(OSD_INVALID_LEVEL));
        result = CR_FAILURE;
        goto cleanup;
    } else if (matches->count >= 1) {
        const STRING_FUZZY_MATCH *const match = Vector_Get(matches, 0);
        level_to_load = (int32_t)(intptr_t)match->value;
        goto matched;
    }

matched:
    if (level_to_load >= 0 && level_to_load < level_table->count) {
        const GF_LEVEL *const level = &level_table->levels[level_to_load];
        GF_OverrideCommand((GF_COMMAND) {
            .action = GF_SELECT_GAME,
            .param = level_to_load,
        });
        Console_Log(GS(OSD_PLAY_LEVEL), level->title);
        result = CR_SUCCESS;
    } else {
        Console_Log(GS(OSD_INVALID_LEVEL));
        result = CR_FAILURE;
    }

cleanup:
    if (matches != NULL) {
        Vector_Free(matches);
        matches = NULL;
    }
    if (source != NULL) {
        Vector_Free(source);
        source = NULL;
    }

    return result;
}

CONSOLE_COMMAND g_Console_Cmd_PlayLevel = {
    .prefix = "play|level",
    .proc = M_Entrypoint,
};
