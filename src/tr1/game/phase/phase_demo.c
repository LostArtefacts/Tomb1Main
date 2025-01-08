#include "game/phase/phase_demo.h"

#include "game/camera.h"
#include "game/effects.h"
#include "game/game.h"
#include "game/game_string.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/item_actions.h"
#include "game/items.h"
#include "game/lara/cheat.h"
#include "game/lara/common.h"
#include "game/lara/hair.h"
#include "game/level.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/phase.h"
#include "game/random.h"
#include "game/room.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/text.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/debug.h>
#include <libtrx/game/fader.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    STATE_RUN,
    STATE_FADE_OUT,
    STATE_INVALID,
} STATE;

static CONFIG m_OldConfig;
static RESUME_INFO m_OldResumeInfo;
static TEXTSTRING *m_DemoModeText = NULL;
static STATE m_State = STATE_RUN;
static FADER m_Fader;

static int32_t m_DemoLevel = -1;
static uint32_t *m_DemoPtr = NULL;

static void M_FadeOut(void);
static bool M_ProcessInput(void);
static int32_t M_ChooseLevel(int32_t demo_num);
static void M_PrepareConfig(void);
static void M_RestoreConfig(void);
static void M_PrepareResumeInfo(void);
static void M_RestoreResumeInfo(void);

static void M_Start(const PHASE_DEMO_ARGS *args);
static void M_End(void);
static PHASE_CONTROL M_ControlRun(int32_t nframes);
static PHASE_CONTROL M_ControlFadeOut(void);
static PHASE_CONTROL M_Control(int32_t nframes);
static void M_Draw(void);

static void M_FadeOut(void)
{
    m_State = STATE_FADE_OUT;
    Fader_Init(&m_Fader, FADER_TRANSPARENT, FADER_BLACK, 0.5);
}

static bool M_ProcessInput(void)
{
    if (m_DemoPtr >= &g_DemoData[DEMO_COUNT_MAX] || (int)*m_DemoPtr == -1) {
        return false;
    }

    union {
        uint32_t any;
        struct {
            // clang-format off
            uint32_t forward:      1;
            uint32_t back:         1;
            uint32_t left:         1;
            uint32_t right:        1;
            uint32_t jump:         1;
            uint32_t draw:         1;
            uint32_t action:       1;
            uint32_t slow:         1;
            uint32_t option:       1;
            uint32_t look:         1;
            uint32_t step_left:    1;
            uint32_t step_right:   1;
            uint32_t roll:         1;
            uint32_t _pad:         7;
            uint32_t menu_confirm: 1;
            uint32_t menu_back:    1;
            uint32_t save:         1;
            uint32_t load:         1;
            // clang-format on
        };
    } demo_input = { .any = *m_DemoPtr };

    // Translate demo inputs (that use TombATI key values) to TR1X inputs.
    g_Input = (INPUT_STATE) {
        0,
        // clang-format off
        .forward      = demo_input.forward,
        .back         = demo_input.back,
        .left         = demo_input.left,
        .right        = demo_input.right,
        .jump         = demo_input.jump,
        .draw         = demo_input.draw,
        .action       = demo_input.action,
        .slow         = demo_input.slow,
        .option       = demo_input.option,
        .look         = demo_input.look,
        .step_left    = demo_input.step_left,
        .step_right   = demo_input.step_right,
        .roll         = demo_input.roll,
        .menu_confirm = demo_input.menu_confirm,
        .menu_back    = demo_input.menu_back,
        .save         = demo_input.save,
        .load         = demo_input.load,
        // clang-format on
    };

    m_DemoPtr++;
    return true;
}

static int32_t M_ChooseLevel(const int32_t demo_num)
{
    bool any_demos = false;
    for (int32_t i = g_GameFlow.first_level_num; i <= g_GameFlow.last_level_num;
         i++) {
        if (g_GameFlow.levels[i].demo) {
            any_demos = true;
        }
    }
    if (!any_demos) {
        return -1;
    }

    if (demo_num >= 0) {
        int32_t j = 0;
        for (int32_t i = g_GameFlow.first_level_num;
             i <= g_GameFlow.last_level_num; i++) {
            if (g_GameFlow.levels[i].demo) {
                if (j == demo_num) {
                    return i;
                }
                j++;
            }
        }
        return -1;
    }

    // pick the next demo
    int16_t level_num = m_DemoLevel;
    do {
        level_num++;
        if (level_num > g_GameFlow.last_level_num) {
            level_num = g_GameFlow.first_level_num;
        }
    } while (!g_GameFlow.levels[level_num].demo);
    return level_num;
}

static void M_PrepareConfig(void)
{
    // Changing certains settings affects negatively the original game demo
    // data, so temporarily turn off all relevant enhancements.
    m_OldConfig = g_Config;
    g_Config.gameplay.enable_enhanced_look = false;
    g_Config.gameplay.enable_tr2_jumping = false;
    g_Config.gameplay.enable_tr2_swimming = false;
    g_Config.gameplay.enable_tr2_swim_cancel = false;
    g_Config.gameplay.enable_wading = false;
    g_Config.gameplay.target_mode = TLM_FULL;
    g_Config.gameplay.fix_bear_ai = false;
}

static void M_RestoreConfig(void)
{
    g_Config = m_OldConfig;
}

static void M_PrepareResumeInfo(void)
{
    RESUME_INFO *resume_info = &g_GameInfo.current[m_DemoLevel];
    m_OldResumeInfo = *resume_info;
    resume_info->flags.available = 1;
    resume_info->flags.got_pistols = 1;
    resume_info->pistol_ammo = 1000;
    resume_info->gun_status = LGS_ARMLESS;
    resume_info->equipped_gun_type = LGT_PISTOLS;
    resume_info->holsters_gun_type = LGT_PISTOLS;
    resume_info->back_gun_type = LGT_UNARMED;
    resume_info->lara_hitpoints = LARA_MAX_HITPOINTS;
}

static void M_RestoreResumeInfo(void)
{
    g_GameInfo.current[m_DemoLevel] = m_OldResumeInfo;
}

static void M_Start(const PHASE_DEMO_ARGS *const args)
{
    m_DemoModeText = Text_Create(0, -16, GS(MISC_DEMO_MODE));
    Text_Flash(m_DemoModeText, 1, 20);
    Text_AlignBottom(m_DemoModeText, 1);
    Text_CentreH(m_DemoModeText, 1);

    if (args != NULL && args->resume_existing) {
        // The demo is already playing and it's to be resumed.
        M_PrepareConfig();
        M_PrepareResumeInfo();
        return;
    }

    m_DemoLevel = M_ChooseLevel(args != NULL ? args->demo_num : -1);
    if (m_DemoLevel == -1) {
        m_State = STATE_INVALID;
        return;
    }

    m_State = STATE_RUN;

    // Remember old inputs in case the demo was forcefully started with some
    // keys pressed. In that case, it should only be stopped if the user
    // presses some other key.
    Input_Update();

    Interpolation_Remember();
    Game_SetIsPlaying(true);

    M_PrepareConfig();
    M_PrepareResumeInfo();

    Fader_Init(&m_Fader, FADER_TRANSPARENT, FADER_TRANSPARENT, 0.0);
    Random_SeedDraw(0xD371F947);
    Random_SeedControl(0xD371F947);

    if (!Level_Initialise(m_DemoLevel)) {
        Shell_ExitSystem("Unable to initialize level");
    }
    g_GameInfo.current_level_type = GFL_DEMO;

    g_OverlayFlag = 1;
    Camera_Initialise();

    m_DemoPtr = g_DemoData;

    ITEM *item = g_LaraItem;
    item->pos.x = *m_DemoPtr++;
    item->pos.y = *m_DemoPtr++;
    item->pos.z = *m_DemoPtr++;
    item->rot.x = *m_DemoPtr++;
    item->rot.y = *m_DemoPtr++;
    item->rot.z = *m_DemoPtr++;
    int16_t room_num = *m_DemoPtr++;

    if (item->room_num != room_num) {
        Item_NewRoom(g_Lara.item_num, room_num);
    }

    const SECTOR *const sector =
        Room_GetSector(item->pos.x, item->pos.y, item->pos.z, &room_num);
    item->floor = Room_GetHeight(sector, item->pos.x, item->pos.y, item->pos.z);

    Random_SeedDraw(0xD371F947);
    Random_SeedControl(0xD371F947);

    // LaraGun() expects request_gun_type to be set only when it
    // really is needed, not at all times.
    // https://github.com/LostArtefacts/TRX/issues/36
    g_Lara.request_gun_type = LGT_UNARMED;
}

static void M_End(void)
{
    Game_SetIsPlaying(false);
    if (m_DemoLevel == -1) {
        return;
    }

    M_RestoreConfig();
    M_RestoreResumeInfo();

    Text_Remove(m_DemoModeText);
}

static PHASE_CONTROL M_ControlRun(int32_t nframes)
{
    Interpolation_Remember();
    CLAMPG(nframes, MAX_FRAMES);

    for (int32_t i = 0; i < nframes; i++) {
        Lara_Cheat_Control();
        if (g_LevelComplete) {
            M_FadeOut();
            goto end;
        }

        if (!M_ProcessInput()) {
            M_FadeOut();
            goto end;
        }
        Game_ProcessInput();

        Item_Control();
        Effect_Control();

        Lara_Control();
        Lara_Hair_Control();

        Camera_Update();
        Sound_ResetAmbient();
        ItemAction_RunActive();
        Sound_UpdateEffects();
        Overlay_BarHealthTimerTick();

        // Discard demo input; check for debounced real keypresses
        Input_Update();
        Shell_ProcessInput();
        if (g_InputDB.toggle_photo_mode) {
            Game_SetIsPlaying(false);
            PHASE *const subphase = Phase_PhotoMode_Create();
            const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
            Phase_PhotoMode_Destroy(subphase);
            Game_SetIsPlaying(true);
            if (gf_cmd.action != GF_NOOP) {
                return (PHASE_CONTROL) {
                    .action = PHASE_ACTION_END,
                    .gf_cmd = gf_cmd,
                };
            }
            return (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        } else if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
            M_FadeOut();
            goto end;
        }
    }

end:
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static PHASE_CONTROL M_ControlFadeOut(void)
{
    Text_Flash(m_DemoModeText, 0, 0);
    Input_Update();
    Shell_ProcessInput();

    if (g_InputDB.menu_confirm || g_InputDB.menu_back
        || !Fader_IsActive(&m_Fader)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static PHASE_CONTROL M_Control(int32_t nframes)
{
    switch (m_State) {
    case STATE_INVALID:
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_EXIT_TO_TITLE },
        };

    case STATE_RUN:
        return M_ControlRun(nframes);

    case STATE_FADE_OUT:
        return M_ControlFadeOut();
    }

    ASSERT_FAIL();
    return (PHASE_CONTROL) {
        .action = PHASE_ACTION_END,
        .gf_cmd = { .action = GF_NOOP },
    };
}

static void M_Draw(void)
{
    if (m_State == STATE_FADE_OUT) {
        Interpolation_Disable();
    }
    Game_Draw(true);
    if (m_State == STATE_FADE_OUT) {
        Interpolation_Enable();
    }

    Output_AnimateTextures();
    Text_Draw();
    Fader_Draw(&m_Fader);
}

PHASER g_DemoPhaser = {
    .start = (PHASER_START)M_Start,
    .end = M_End,
    .control = M_Control,
    .draw = M_Draw,
    .wait = NULL,
};
