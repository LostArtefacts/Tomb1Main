#include "game/console/registry.h"
#include "game/game_flow.h"
#include "game/lara/cheat.h"
#include "strings.h"

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *ctx);

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *const ctx)
{
    if (!String_IsEmpty(ctx->args)) {
        return CR_BAD_INVOCATION;
    }

    if (GF_GetCurrentLevel() == nullptr
        || GF_GetCurrentLevel()->type == GFL_TITLE) {
        return CR_UNAVAILABLE;
    }

    Lara_Cheat_EndLevel();
    return CR_SUCCESS;
}

REGISTER_CONSOLE_COMMAND("end-?level|next-?level", M_Entrypoint)
