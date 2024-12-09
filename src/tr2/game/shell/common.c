#include "game/shell/common.h"

#include "config.h"
#include "decomp/decomp.h"
#include "decomp/fmv.h"
#include "decomp/savegame.h"
#include "game/background.h"
#include "game/clock.h"
#include "game/console/common.h"
#include "game/demo.h"
#include "game/game_string.h"
#include "game/gamebuf.h"
#include "game/gameflow.h"
#include "game/gameflow/reader.h"
#include "game/input.h"
#include "game/music.h"
#include "game/sound.h"
#include "game/text.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/enum_map.h>
#include <libtrx/game/ui/common.h>
#include <libtrx/memory.h>

#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>

#define GAMEBUF_MEM_CAP 0x380000

static const char *m_CurrentGameflowPath = "cfg/TR2X_gameflow.json5";

// TODO: refactor the hell out of me
void __cdecl Shell_Main(void)
{
    GameString_Init();
    EnumMap_Init();
    Config_Init();
    Text_Init();
    UI_Init();
    Console_Init();

    Clock_Init();
    Sound_Init();
    Music_Init();
    Input_Init();

    Config_Read();
    if (!S_InitialiseSystem()) {
        return;
    }

    if (!GF_LoadScriptFile("data\\tombPC.dat")) {
        Shell_ExitSystem("GameMain: could not load script file");
        return;
    }

    if (!GF_N_Load(m_CurrentGameflowPath)) {
        Shell_ExitSystem("GameMain: could not load new script file");
        return;
    }

    InitialiseStartInfo();
    S_FrontEndCheck();

    GameBuf_Init(GAMEBUF_MEM_CAP);

    g_IsVidModeLock = 1;

    S_DisplayPicture("data\\legal.pcx", 0);
    S_InitialisePolyList(0);
    S_CopyBufferToScreen();
    S_OutputPolyList();
    S_DumpScreen();
    Shell_ProcessEvents();
    FadeToPal(30, g_GamePalette8);
    S_Wait(180, true);
    S_FadeToBlack();
    S_DontDisplayPicture();
    g_IsVidModeLock = 0;

    const bool is_frontend_fail = GF_DoFrontendSequence();
    if (g_IsGameToExit) {
        Config_Write();
        return;
    }

    if (is_frontend_fail) {
        Shell_ExitSystem("GameMain: failed in GF_DoFrontendSequence()");
        return;
    }

    S_FadeToBlack();
    int16_t gf_option = g_GameFlow.first_option;
    g_NoInputCounter = 0;

    bool is_loop_continued = true;
    while (is_loop_continued) {
        const int16_t gf_dir = gf_option & 0xFF00;
        const int16_t gf_param = gf_option & 0x00FF;

        switch (gf_dir) {
        case GFD_START_GAME:
            if (g_GameFlow.single_level >= 0) {
                gf_option =
                    GF_DoLevelSequence(g_GameFlow.single_level, GFL_NORMAL);
            } else {
                if (gf_param > g_GameFlow.num_levels) {
                    Shell_ExitSystemFmt(
                        "GameMain: STARTGAME with invalid level number (%d)",
                        gf_param);
                    return;
                }
                gf_option = GF_DoLevelSequence(gf_param, GFL_NORMAL);
            }
            break;

        case GFD_START_SAVED_GAME:
            S_LoadGame(&g_SaveGame, sizeof(SAVEGAME_INFO), gf_param);
            if (g_SaveGame.current_level > g_GameFlow.num_levels) {
                Shell_ExitSystemFmt(
                    "GameMain: STARTSAVEDGAME with invalid level number (%d)",
                    g_SaveGame.current_level);
                return;
            }
            gf_option = GF_DoLevelSequence(g_SaveGame.current_level, GFL_SAVED);
            break;

        case GFD_START_CINE:
            Game_Cutscene_Start(gf_param);
            gf_option = GFD_EXIT_TO_TITLE;
            break;

        case GFD_START_DEMO:
            gf_option = Demo_Control(-1);
            break;

        case GFD_LEVEL_COMPLETE:
            gf_option = LevelCompleteSequence();
            break;

        case GFD_EXIT_TO_TITLE:
        case GFD_EXIT_TO_OPTION:
            if (g_GameFlow.title_disabled) {
                if (g_GameFlow.title_replace < 0
                    || g_GameFlow.title_replace == GFD_EXIT_TO_TITLE) {
                    Shell_ExitSystem(
                        "GameMain Failed: Title disabled & no replacement");
                    return;
                }
                gf_option = g_GameFlow.title_replace;
            } else {
                gf_option = TitleSequence();
                g_GF_StartGame = 1;
            }
            break;

        default:
            is_loop_continued = false;
            break;
        }
    }

    Config_Write();
}

void __cdecl Shell_Shutdown(void)
{
    GameString_Shutdown();
    Console_Shutdown();
    BGND_Free();
    RenderFinish(false);
    WinVidFinish();
    WinVidHideGameWindow();
    Text_Shutdown();
    UI_Shutdown();
    GameBuf_Shutdown();
    Config_Shutdown();
}

const char *Shell_GetGameflowPath(void)
{
    return m_CurrentGameflowPath;
}

// TODO: try to call this function in a single place after introducing phasers.
void Shell_ProcessEvents(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT:
            g_IsMessageLoopClosed = true;
            g_IsGameToExit = true;
            g_StopInventory = true;
            break;

        case SDL_KEYDOWN: {
            // NOTE: This normally would get handled by Input_Update,
            // but by the time Input_Update gets ran, we may already have lost
            // some keypresses if the player types really fast, so we need to
            // react sooner.
            if (!FMV_IsPlaying() && !Console_IsOpened()
                && Input_IsPressed(
                    INPUT_BACKEND_KEYBOARD, g_Config.input.keyboard_layout,
                    INPUT_ROLE_ENTER_CONSOLE)) {
                Console_Open();
            } else {
                UI_HandleKeyDown(event.key.keysym.sym);
            }
            break;
        }

        case SDL_KEYUP:
            // NOTE: needs special handling on Windows -
            // SDL_SCANCODE_PRINTSCREEN is not sufficient to react to this.
            if (event.key.keysym.sym == SDLK_PRINTSCREEN) {
                Screenshot_Make(g_Config.rendering.screenshot_format);
            }
            break;

        case SDL_TEXTEDITING:
            UI_HandleTextEdit(event.text.text);
            break;

        case SDL_TEXTINPUT:
            UI_HandleTextEdit(event.text.text);
            break;

        case SDL_CONTROLLERDEVICEADDED:
        case SDL_JOYDEVICEADDED:
            Input_InitController();
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_JOYDEVICEREMOVED:
            Input_ShutdownController();
            break;
        }
    }
}

SDL_Window *Shell_GetWindow(void)
{
    return g_SDLWindow;
}
