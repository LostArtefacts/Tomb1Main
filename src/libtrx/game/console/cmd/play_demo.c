#include "game/console/common.h"
#include "game/console/registry.h"
#include "game/demo.h"
#include "game/game_flow/common.h"
#include "game/game_string.h"
#include "strings.h"

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *ctx);

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *const ctx)
{
    int32_t demo_to_load = -1;
    const GF_LEVEL_TABLE *const level_table = GF_GetLevelTable(GFLT_DEMOS);
    if (String_IsEmpty(ctx->args)) {
        demo_to_load = Demo_ChooseLevel(-1);
    } else if (String_ParseInteger(ctx->args, &demo_to_load)) {
        demo_to_load--;
    } else {
        return CR_BAD_INVOCATION;
    }

    if (demo_to_load < 0 || demo_to_load >= level_table->count) {
        Console_Log(GS(OSD_INVALID_DEMO));
        return CR_FAILURE;
    }
    const GF_LEVEL *const level = &level_table->levels[demo_to_load];
    GF_OverrideCommand((GF_COMMAND) {
        .action = GF_START_DEMO,
        .param = demo_to_load,
    });
    Console_Log(GS(OSD_PLAY_DEMO), level->num + 1);
    return CR_SUCCESS;
}

REGISTER_CONSOLE_COMMAND("demo", M_Entrypoint)
