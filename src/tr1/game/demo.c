#include "game/demo.h"

#include "game/camera.h"
#include "game/effects.h"
#include "game/game.h"
#include "game/game_flow.h"
#include "game/game_string.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/item_actions.h"
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
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/log.h>

#define MODIFY_CONFIG()                                                        \
    PROCESS_CONFIG(gameplay.enable_enhanced_look, false);                      \
    PROCESS_CONFIG(gameplay.enable_tr2_jumping, false);                        \
    PROCESS_CONFIG(gameplay.enable_tr2_swimming, false);                       \
    PROCESS_CONFIG(gameplay.enable_tr2_swim_cancel, false);                    \
    PROCESS_CONFIG(gameplay.enable_wading, false);                             \
    PROCESS_CONFIG(gameplay.target_mode, TLM_FULL);                            \
    PROCESS_CONFIG(gameplay.fix_bear_ai, false);

typedef struct {
    uint32_t *demo_ptr;
    GAME_FLOW_LEVEL *level;
    CONFIG old_config;
    RESUME_INFO old_resume_info;
    TEXTSTRING *text;
} M_PRIV;

static int32_t m_LastDemoNum = 0;
static M_PRIV m_Priv;

static void M_PrepareConfig(M_PRIV *const p);
static void M_RestoreConfig(M_PRIV *const p);
static void M_PrepareResumeInfo(M_PRIV *const p);
static void M_RestoreResumeInfo(M_PRIV *const p);
static bool M_ProcessInput(M_PRIV *const p);

static void M_PrepareConfig(M_PRIV *const p)
{
    // Changing certains settings affects negatively the original game demo
    // data, so temporarily turn off all relevant enhancements.
    p->old_config = g_Config;
#undef PROCESS_CONFIG
#define PROCESS_CONFIG(var, value) g_Config.var = value;
    MODIFY_CONFIG();
}

static void M_RestoreConfig(M_PRIV *const p)
{
#undef PROCESS_CONFIG
#define PROCESS_CONFIG(var, value) g_Config.var = p->old_config.var;
    MODIFY_CONFIG();
}

static void M_PrepareResumeInfo(M_PRIV *const p)
{
    RESUME_INFO *const resume_info = GF_GetResumeInfo(p->level);
    p->old_resume_info = *resume_info;
    resume_info->flags.available = 1;
    resume_info->flags.costume = 0;
    resume_info->num_medis = 0;
    resume_info->num_big_medis = 0;
    resume_info->num_scions = 0;
    resume_info->flags.got_pistols = 1;
    resume_info->flags.got_shotgun = 0;
    resume_info->flags.got_magnums = 0;
    resume_info->flags.got_uzis = 0;
    resume_info->pistol_ammo = 1000;
    resume_info->shotgun_ammo = 0;
    resume_info->magnum_ammo = 0;
    resume_info->uzi_ammo = 0;
    resume_info->gun_status = LGS_ARMLESS;
    resume_info->equipped_gun_type = LGT_PISTOLS;
    resume_info->holsters_gun_type = LGT_PISTOLS;
    resume_info->back_gun_type = LGT_UNARMED;
    resume_info->lara_hitpoints = LARA_MAX_HITPOINTS;
}

static void M_RestoreResumeInfo(M_PRIV *const p)
{
    RESUME_INFO *const resume_info = GF_GetResumeInfo(p->level);
    *resume_info = p->old_resume_info;
}

static bool M_ProcessInput(M_PRIV *const p)
{
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
    } demo_input = { .any = *p->demo_ptr };

    if ((int32_t)demo_input.any == -1) {
        return false;
    }

    // Translate demo inputs (that use TombATI key values) to TR1X inputs.
    g_Input = (INPUT_STATE) {
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

    p->demo_ptr++;
    return true;
}

bool Demo_Start(const int32_t level_num)
{
    M_PRIV *const p = &m_Priv;
    p->level = GF_GetLevel(level_num, GFL_DEMO);

    M_PrepareConfig(p);
    M_PrepareResumeInfo(p);

    // Remember old inputs in case the demo was forcefully started with some
    // keys pressed. In that case, it should only be stopped if the user
    // presses some other key.
    Input_Update();

    Interpolation_Remember();

    Random_SeedDraw(0xD371F947);
    Random_SeedControl(0xD371F947);

    if (!Level_Initialise(p->level)) {
        return false;
    }
    if (g_DemoData == NULL) {
        LOG_ERROR("Level '%s' has no demo data", p->level->path);
        return false;
    }
    g_GameInfo.current_level_type = GFL_DEMO;

    g_OverlayFlag = 1;
    Camera_Initialise();
    p->demo_ptr = g_DemoData;

    ITEM *const lara_item = Lara_GetItem();
    lara_item->pos.x = *p->demo_ptr++;
    lara_item->pos.y = *p->demo_ptr++;
    lara_item->pos.z = *p->demo_ptr++;
    lara_item->rot.x = *p->demo_ptr++;
    lara_item->rot.y = *p->demo_ptr++;
    lara_item->rot.z = *p->demo_ptr++;
    int16_t room_num = *p->demo_ptr++;

    if (lara_item->room_num != room_num) {
        Item_NewRoom(g_Lara.item_num, room_num);
    }

    const SECTOR *const sector = Room_GetSector(
        lara_item->pos.x, lara_item->pos.y, lara_item->pos.z, &room_num);
    lara_item->floor = Room_GetHeight(
        sector, lara_item->pos.x, lara_item->pos.y, lara_item->pos.z);

    Random_SeedDraw(0xD371F947);
    Random_SeedControl(0xD371F947);

    // LaraGun() expects request_gun_type to be set only when it
    // really is needed, not at all times.
    // https://github.com/LostArtefacts/TRX/issues/36
    g_Lara.request_gun_type = LGT_UNARMED;

    p->text = Text_Create(0, -16, GS(MISC_DEMO_MODE));
    Text_Flash(p->text, true, 20);
    Text_AlignBottom(p->text, true);
    Text_CentreH(p->text, true);
    g_GameInfo.showing_demo = true;
    return true;
}

void Demo_End(void)
{
    M_PRIV *const p = &m_Priv;
    M_RestoreConfig(p);
    M_RestoreResumeInfo(p);
    Text_Remove(p->text);
    p->text = NULL;
    g_GameInfo.showing_demo = false;
}

void Demo_Pause(void)
{
    M_PRIV *const p = &m_Priv;
    M_RestoreConfig(p);
    M_RestoreResumeInfo(p);
    Text_Hide(p->text, true);
    g_GameInfo.showing_demo = false;
}

void Demo_Unpause(void)
{
    M_PRIV *const p = &m_Priv;
    M_PrepareConfig(p);
    M_PrepareResumeInfo(p);
    Text_Hide(p->text, false);
    g_GameInfo.showing_demo = true;
}

int32_t Demo_ChooseLevel(const int32_t demo_num)
{
    M_PRIV *const p = &m_Priv;
    if (GF_GetDemoCount() <= 0) {
        return -1;
    } else if (demo_num < 0 || demo_num >= GF_GetDemoCount()) {
        return (m_LastDemoNum++) % GF_GetDemoCount();
    } else {
        return demo_num;
    }
}

GAME_FLOW_COMMAND Demo_Control(void)
{
    Interpolation_Remember();
    M_PRIV *const p = &m_Priv;

    Input_Update();
    Shell_ProcessInput();

    if (g_InputDB.pause) {
        PHASE *const subphase = Phase_Pause_Create();
        const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
        Phase_Pause_Destroy(subphase);
        return gf_cmd;
    } else if (g_InputDB.toggle_photo_mode) {
        PHASE *const subphase = Phase_PhotoMode_Create();
        const GAME_FLOW_COMMAND gf_cmd = PhaseExecutor_Run(subphase);
        Phase_PhotoMode_Destroy(subphase);
        return gf_cmd;
    }

    if (g_LevelComplete || g_InputDB.menu_confirm || g_InputDB.menu_back) {
        return (GAME_FLOW_COMMAND) {
            .action = GF_LEVEL_COMPLETE,
            .param = p->level->num,
        };
    }
    g_Input.any = 0;
    g_InputDB.any = 0;

    if (!M_ProcessInput(p)) {
        return (GAME_FLOW_COMMAND) {
            .action = GF_LEVEL_COMPLETE,
            .param = p->level->num,
        };
    }
    Lara_Cheat_Control();

    Game_ProcessInput();

    Output_ResetDynamicLights();

    Item_Control();
    Effect_Control();

    Lara_Control();
    Lara_Hair_Control();

    Camera_Update();
    Sound_ResetAmbient();
    ItemAction_RunActive();
    Sound_UpdateEffects();
    Overlay_BarHealthTimerTick();
    Output_AnimateTextures(1);

    return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
}

void Demo_StopFlashing(void)
{
    M_PRIV *const p = &m_Priv;
    Text_Flash(p->text, false, 0);
}
