#include "game/console/cmd/exit_game.h"

#include "game/game_flow/common.h"
#include "strings.h"

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *ctx);

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *const ctx)
{
    if (!String_IsEmpty(ctx->args)) {
        return CR_BAD_INVOCATION;
    }

    GF_OverrideCommand((GF_COMMAND) { .action = GF_EXIT_GAME });
    return CR_SUCCESS;
}

CONSOLE_COMMAND g_Console_Cmd_ExitGame = {
    .prefix = "exit|quit",
    .proc = M_Entrypoint,
};
