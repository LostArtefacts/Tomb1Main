#include "game/console/cmd/exit_to_title.h"

#include "game/game_flow/common.h"
#include "strings.h"

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *ctx);

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *const ctx)
{
    if (!String_IsEmpty(ctx->args)) {
        return CR_BAD_INVOCATION;
    }

    GameFlow_OverrideCommand(
        (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE });
    return CR_SUCCESS;
}

CONSOLE_COMMAND g_Console_Cmd_ExitToTitle = {
    .prefix = "title",
    .proc = M_Entrypoint,
};
