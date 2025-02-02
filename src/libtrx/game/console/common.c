#include "game/console/common.h"

#include "./internal.h"
#include "debug.h"
#include "game/console/registry.h"
#include "game/game_string.h"
#include "game/ui/widgets/console.h"
#include "log.h"
#include "memory.h"
#include "strings.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static bool m_IsOpened = false;
static UI_WIDGET *m_Console;

void Console_Init(void)
{
    m_Console = UI_Console_Create();
    Console_History_Init();
}

void Console_Shutdown(void)
{
    if (m_Console != nullptr) {
        m_Console->free(m_Console);
        m_Console = nullptr;
    }

    Console_History_Shutdown();
    Console_Registry_Shutdown();

    m_IsOpened = false;
}

void Console_Open(void)
{
    if (m_IsOpened) {
        UI_Console_HandleClose(m_Console);
    }
    m_IsOpened = true;
    UI_Console_HandleOpen(m_Console);
}

void Console_Close(void)
{
    UI_Console_HandleClose(m_Console);
    m_IsOpened = false;
}

bool Console_IsOpened(void)
{
    return m_IsOpened;
}

void Console_ScrollLogs(void)
{
    UI_Console_ScrollLogs(m_Console);
}

int32_t Console_GetVisibleLogCount(void)
{
    return UI_Console_GetVisibleLogCount(m_Console);
}

int32_t Console_GetMaxLogCount(void)
{
    return UI_Console_GetMaxLogCount(m_Console);
}

void Console_Log(const char *fmt, ...)
{
    ASSERT(fmt != nullptr);

    va_list va;

    va_start(va, fmt);
    const size_t text_length = vsnprintf(nullptr, 0, fmt, va);
    char *text = Memory_Alloc(text_length + 1);
    va_end(va);

    va_start(va, fmt);
    vsnprintf(text, text_length + 1, fmt, va);
    va_end(va);

    LOG_INFO("%s", text);
    UI_Console_HandleLog(m_Console, text);
    Memory_FreePointer(&text);
}

COMMAND_RESULT Console_Eval(const char *const cmdline)
{
    LOG_INFO("executing command: %s", cmdline);

    const CONSOLE_COMMAND *const matching_cmd = Console_Registry_Get(cmdline);
    if (matching_cmd == nullptr) {
        Console_Log(GS(OSD_UNKNOWN_COMMAND), cmdline);
        return CR_BAD_INVOCATION;
    }

    char *prefix = Memory_DupStr(cmdline);
    char *args = "";
    char *space = strchr(prefix, ' ');
    if (space != nullptr) {
        *space = '\0';
        args = space + 1;
    }

    const COMMAND_CONTEXT ctx = {
        .cmd = matching_cmd,
        .prefix = prefix,
        .args = args,
    };
    ASSERT(matching_cmd->proc != nullptr);
    const COMMAND_RESULT result = matching_cmd->proc(&ctx);
    Memory_FreePointer(&prefix);

    switch (result) {
    case CR_BAD_INVOCATION:
        Console_Log(GS(OSD_COMMAND_BAD_INVOCATION), cmdline);
        break;

    case CR_UNAVAILABLE:
        Console_Log(GS(OSD_COMMAND_UNAVAILABLE));
        break;

    case CR_SUCCESS:
    case CR_FAILURE:
        // The commands themselves are responsible for handling logging in
        // these scenarios.
        break;
    }
    return result;
}

void Console_Draw(void)
{
    if (m_Console == nullptr) {
        return;
    }

    Console_ScrollLogs();

    if (Console_IsOpened() || Console_GetVisibleLogCount() > 0) {
        Console_DrawBackdrop();
    }

    m_Console->draw(m_Console);
}
