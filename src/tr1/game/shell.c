#include "game/shell.h"

#include "config.h"
#include "game/clock.h"
#include "game/console/common.h"
#include "game/fmv.h"
#include "game/game.h"
#include "game/game_string.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/level.h"
#include "game/music.h"
#include "game/option.h"
#include "game/output.h"
#include "game/phase/phase.h"
#include "game/savegame.h"
#include "game/screen.h"
#include "game/sound.h"
#include "game/text.h"
#include "global/types.h"
#include "global/vars.h"
#include "specific/s_shell.h"

#include <libtrx/enum_map.h>
#include <libtrx/filesystem.h>
#include <libtrx/game/gamebuf.h>
#include <libtrx/game/ui/common.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GAMEBUF_MEM_CAP 0x8000000
#define LEVEL_TITLE_SIZE 25
#define TIMESTAMP_SIZE 20

static const char m_TR1XGameflowPath[] = "cfg/TR1X_gameflow.json5";
static const char m_TR1XGameflowGoldPath[] = "cfg/TR1X_gameflow_ub.json5";
static const char m_TR1XGameflowDemoPath[] = "cfg/TR1X_gameflow_demo_pc.json5";

static const char *m_CurrentGameflowPath;

void Shell_Init(const char *gameflow_path)
{
    Text_Init();
    UI_Init();

    Clock_Init();
    Sound_Init();
    Music_Init();
    Input_Init();

    Config_Read();

    S_Shell_CreateWindow();
    S_Shell_Init();

    if (!Output_Init()) {
        Shell_ExitSystem("Could not initialise video system");
        return;
    }
    Screen_Init();

    if (!GameFlow_LoadFromFile(gameflow_path)) {
        Shell_ExitSystemFmt("Unable to load gameflow file: %s", gameflow_path);
        return;
    }
    Savegame_Init();
    Savegame_ScanSavedGames();
    Savegame_HighlightNewestSlot();
    GameBuf_Init(GAMEBUF_MEM_CAP);
    Console_Init();
}

void Shell_Shutdown(void)
{
    Console_Shutdown();
    GameBuf_Shutdown();
    Savegame_Shutdown();
    GameFlow_Shutdown();

    Output_Shutdown();
    Input_Shutdown();
    Music_Shutdown();
    Sound_Shutdown();
    UI_Shutdown();
    Text_Shutdown();
    Config_Shutdown();
    Log_Shutdown();
}

const char *Shell_GetGameflowPath(void)
{
    return m_CurrentGameflowPath;
}

void Shell_Main(void)
{
    m_CurrentGameflowPath = m_TR1XGameflowPath;

    char **args = NULL;
    int arg_count = 0;
    S_Shell_GetCommandLine(&arg_count, &args);
    for (int i = 0; i < arg_count; i++) {
        if (!strcmp(args[i], "-gold")) {
            m_CurrentGameflowPath = m_TR1XGameflowGoldPath;
        }
        if (!strcmp(args[i], "-demo_pc")) {
            m_CurrentGameflowPath = m_TR1XGameflowDemoPath;
        }
    }
    for (int i = 0; i < arg_count; i++) {
        Memory_FreePointer(&args[i]);
    }
    Memory_FreePointer(&args);

    GameString_Init();
    EnumMap_Init();
    Config_Init();

    Shell_Init(m_CurrentGameflowPath);

    GAMEFLOW_COMMAND command = { .action = GF_EXIT_TO_TITLE };
    bool intro_played = false;

    g_GameInfo.current_save_slot = -1;
    bool loop_continue = true;
    while (loop_continue) {
        LOG_INFO("action=%d param=%d", command.action, command.param);

        switch (command.action) {
        case GF_START_GAME: {
            GAMEFLOW_LEVEL_TYPE level_type = GFL_NORMAL;
            if (g_GameFlow.levels[command.param].level_type == GFL_BONUS) {
                level_type = GFL_BONUS;
            }
            command = GameFlow_InterpretSequence(command.param, level_type);
            break;
        }

        case GF_START_SAVED_GAME: {
            int16_t level_num = Savegame_GetLevelNumber(command.param);
            if (level_num < 0) {
                LOG_ERROR("Corrupt save file!");
                command = (GAMEFLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            } else {
                g_GameInfo.current_save_slot = command.param;
                command = GameFlow_InterpretSequence(level_num, GFL_SAVED);
            }
            break;
        }

        case GF_RESTART_GAME: {
            command = GameFlow_InterpretSequence(command.param, GFL_RESTART);
            break;
        }

        case GF_SELECT_GAME: {
            command = GameFlow_InterpretSequence(command.param, GFL_SELECT);
            break;
        }

        case GF_STORY_SO_FAR: {
            command = GameFlow_PlayAvailableStory(command.param);
            break;
        }

        case GF_START_CINE:
            command = GameFlow_InterpretSequence(command.param, GFL_CUTSCENE);
            break;

        case GF_START_DEMO:
            Phase_Set(PHASE_DEMO, NULL);
            command = Phase_Run();
            break;

        case GF_LEVEL_COMPLETE:
            command = (GAMEFLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
            break;

        case GF_EXIT_TO_TITLE:
            g_GameInfo.current_save_slot = -1;
            if (!intro_played) {
                GameFlow_InterpretSequence(
                    g_GameFlow.title_level_num, GFL_TITLE);
                intro_played = true;
            }

            Savegame_InitCurrentInfo();
            if (!Level_Initialise(g_GameFlow.title_level_num)) {
                command = (GAMEFLOW_COMMAND) { .action = GF_EXIT_GAME };
                break;
            }
            g_GameInfo.current_level_type = GFL_TITLE;

            command = Game_MainMenu();
            break;

        case GF_EXIT_GAME:
            loop_continue = false;
            break;

        case GF_START_GYM:
            command = GameFlow_InterpretSequence(command.param, GFL_GYM);
            break;

        default:
            Shell_ExitSystemFmt(
                "MAIN: Unknown action %x %d", command.action, command.param);
            return;
        }
    }

    Config_Write();
    EnumMap_Shutdown();
    GameString_Shutdown();
}

void Shell_ProcessInput(void)
{
    if (g_InputDB.screenshot) {
        Screenshot_Make(g_Config.screenshot_format);
    }

    if (g_InputDB.toggle_bilinear_filter) {
        g_Config.rendering.texture_filter =
            (g_Config.rendering.texture_filter + 1) % GFX_TF_NUMBER_OF;

        switch (g_Config.rendering.texture_filter) {
        case GFX_TF_NN:
            Console_Log(GS(OSD_TEXTURE_FILTER_SET), GS(OSD_TEXTURE_FILTER_NN));
            break;
        case GFX_TF_BILINEAR:
            Console_Log(
                GS(OSD_TEXTURE_FILTER_SET), GS(OSD_TEXTURE_FILTER_BILINEAR));
            break;
        case GFX_TF_NUMBER_OF:
            break;
        }

        Config_Write();
    }

    if (g_InputDB.toggle_perspective_filter) {
        g_Config.rendering.enable_perspective_filter ^= true;
        Console_Log(
            g_Config.rendering.enable_perspective_filter
                ? GS(OSD_PERSPECTIVE_FILTER_ON)
                : GS(OSD_PERSPECTIVE_FILTER_OFF));
        Config_Write();
    }

    if (g_InputDB.toggle_fps_counter) {
        g_Config.rendering.enable_fps_counter ^= true;
        Console_Log(
            g_Config.rendering.enable_fps_counter ? GS(OSD_FPS_COUNTER_ON)
                                                  : GS(OSD_FPS_COUNTER_OFF));
        Config_Write();
    }

    if (g_InputDB.toggle_fullscreen) {
        S_Shell_ToggleFullscreen();
    }

    if (g_InputDB.turbo_cheat) {
        Clock_CycleTurboSpeed(!g_Input.slow);
    }
}
