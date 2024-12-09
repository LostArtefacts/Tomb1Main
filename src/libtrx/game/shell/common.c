#include "debug.h"
#include "game/shell.h"
#include "log.h"
#include "memory.h"

static void M_ShowFatalError(const char *const message)
{
    LOG_ERROR("%s", message);
    SDL_Window *const window = Shell_GetWindow();
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR, "Tomb Raider Error", message, window);
    Shell_Terminate(1);
}

void Shell_Terminate(int32_t exit_code)
{
    Shell_Shutdown();

    SDL_Window *const window = Shell_GetWindow();
    if (window != NULL) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    exit(exit_code);
}

void Shell_ExitSystem(const char *message)
{
    M_ShowFatalError(message);
    Shell_Shutdown();
}

void Shell_ExitSystemFmt(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int32_t size = vsnprintf(NULL, 0, fmt, va) + 1;
    char *message = Memory_Alloc(size);
    va_end(va);

    va_start(va, fmt);
    vsnprintf(message, size, fmt, va);
    va_end(va);

    Shell_ExitSystem(message);

    Memory_FreePointer(&message);
}

int32_t Shell_GetCurrentDisplayWidth(void)
{
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    return dm.w;
}

int32_t Shell_GetCurrentDisplayHeight(void)
{
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    return dm.h;
}

void Shell_GetWindowSize(int32_t *const out_width, int32_t *const out_height)
{
    ASSERT(out_width != NULL);
    ASSERT(out_height != NULL);
    SDL_Window *const window = Shell_GetWindow();
    if (window == NULL) {
        *out_width = -1;
        *out_height = -1;
    } else {
        SDL_GetWindowSize(window, out_width, out_height);
    }
}
