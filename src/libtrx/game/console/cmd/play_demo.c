#include "game/console/cmd/play_demo.h"

#include "game/game_flow/common.h"
#include "game/game_string.h"
#include "strings.h"

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *ctx);

static COMMAND_RESULT M_Entrypoint(const COMMAND_CONTEXT *const ctx)
{
    int32_t demo_to_load = -1;
    if (String_ParseInteger(ctx->args, &demo_to_load)) {
        demo_to_load--;
        if (demo_to_load >= 0 && demo_to_load < GF_GetDemoCount()) {
            GF_OverrideCommand((GAME_FLOW_COMMAND) {
                .action = GF_START_DEMO,
                .param = demo_to_load,
            });
            return CR_SUCCESS;
        } else {
            Console_Log(GS(OSD_INVALID_DEMO));
            return CR_FAILURE;
        }
    } else if (String_IsEmpty(ctx->args)) {
        GF_OverrideCommand(
            (GAME_FLOW_COMMAND) { .action = GF_START_DEMO, .param = -1 });
        return CR_SUCCESS;
    } else {
        return CR_BAD_INVOCATION;
    }
}

CONSOLE_COMMAND g_Console_Cmd_PlayDemo = {
    .prefix = "demo",
    .proc = M_Entrypoint,
};
