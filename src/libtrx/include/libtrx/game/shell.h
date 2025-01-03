#pragma once

#include <SDL2/SDL.h>
#include <stdint.h>

extern void Shell_Shutdown(void);
extern SDL_Window *Shell_GetWindow(void);

void Shell_Setup(void);
void Shell_Terminate(int32_t exit_code);
void Shell_ExitSystem(const char *message);
void Shell_ExitSystemFmt(const char *fmt, ...);

int32_t Shell_GetCurrentDisplayWidth(void);
int32_t Shell_GetCurrentDisplayHeight(void);
void Shell_GetWindowSize(int32_t *out_width, int32_t *out_height);

extern const char *Shell_GetConfigPath(void);
extern const char *Shell_GetGameflowPath(void);
extern void Shell_ProcessInput(void);
extern void Shell_ProcessEvents(void);
