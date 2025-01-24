#include "game/demo.h"

#include "game/camera.h"
#include "game/game.h"
#include "game/game_flow.h"
#include "game/game_string.h"
#include "game/input.h"
#include "game/items.h"
#include "game/lara/cheat.h"
#include "game/level.h"
#include "game/music.h"
#include "game/overlay.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "game/stats.h"
#include "game/text.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/debug.h>
#include <libtrx/log.h>

typedef struct {
    GAME_FLOW_LEVEL *level;
    TEXTSTRING *text;

    struct {
        bool bonus_flag;
    } old_config;
    START_INFO old_start;
} M_PRIV;

static int32_t m_LastDemoNum = 0;
static M_PRIV m_Priv;

static INPUT_STATE m_OldDemoInputDB = {};

static void M_PrepareConfig(M_PRIV *p);
static void M_RestoreConfig(M_PRIV *p);
static void M_PrepareStartInfo(M_PRIV *p);
static void M_RestoreStartInfo(M_PRIV *p);

static void M_PrepareConfig(M_PRIV *const p)
{
    p->old_config.bonus_flag = g_SaveGame.bonus_flag;
    g_SaveGame.bonus_flag = false;
}

static void M_RestoreConfig(M_PRIV *const p)
{
    g_SaveGame.bonus_flag = p->old_config.bonus_flag;
}

static void M_PrepareStartInfo(M_PRIV *const p)
{
    START_INFO *const start = GF_GetResumeInfo(p->level);
    p->old_start = *start;
    start->available = 1;
    start->has_pistols = 1;
    start->pistol_ammo = 1000;
    start->gun_status = LGS_ARMLESS;
    start->gun_type = LGT_PISTOLS;
}

static void M_RestoreStartInfo(M_PRIV *const p)
{
    START_INFO *const start = GF_GetResumeInfo(p->level);
    *start = p->old_start;
}

bool Demo_GetInput(void)
{
    M_PRIV *const p = &m_Priv;
    if (g_DemoCount == 0) {
        m_OldDemoInputDB = (INPUT_STATE) {};
    }
    if (g_DemoCount >= MAX_DEMO_SIZE) {
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
            uint32_t _pad:         6;
            uint32_t use_flare:    1;
            uint32_t menu_confirm: 1;
            uint32_t menu_back:    1;
            uint32_t save:         1;
            uint32_t load:         1;
            // clang-format on
        };
    } demo_input = { .any = g_DemoPtr[g_DemoCount] };

    if ((int32_t)demo_input.any == -1) {
        return false;
    }

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
        .use_flare        = demo_input.use_flare,
        .menu_confirm = demo_input.menu_confirm,
        .menu_back    = demo_input.menu_back,
        .save         = demo_input.save,
        .load         = demo_input.load,
        // clang-format on
    };

    g_InputDB.any = g_Input.any & ~m_OldDemoInputDB.any;
    m_OldDemoInputDB = g_Input;
    g_DemoCount++;
    return true;
}

bool Demo_Start(const int32_t level_num)
{
    M_PRIV *const p = &m_Priv;
    p->level = GF_GetLevel(level_num, GFL_DEMO);
    ASSERT(p->level != NULL);

    M_PrepareConfig(p);
    M_PrepareStartInfo(p);

    Random_SeedDraw(0xD371F947);
    Random_SeedControl(0xD371F947);
    if (!Level_Initialise(level_num, GFL_DEMO)) {
        return false;
    }

    g_LevelComplete = false;
    if (!g_IsDemoLoaded) {
        LOG_ERROR("Level '%s' has no demo data", p->level->path);
        return false;
    }

    g_LaraItem->pos.x = g_DemoPtr[0];
    g_LaraItem->pos.y = g_DemoPtr[1];
    g_LaraItem->pos.z = g_DemoPtr[2];
    g_LaraItem->rot.x = g_DemoPtr[3];
    g_LaraItem->rot.y = g_DemoPtr[4];
    g_LaraItem->rot.z = g_DemoPtr[5];
    int16_t room_num = g_DemoPtr[6];
    if (g_LaraItem->room_num != room_num) {
        Item_NewRoom(g_Lara.item_num, room_num);
    }

    const SECTOR *const sector = Room_GetSector(
        g_LaraItem->pos.x, g_LaraItem->pos.y, g_LaraItem->pos.z, &room_num);
    g_LaraItem->floor = Room_GetHeight(
        sector, g_LaraItem->pos.x, g_LaraItem->pos.y, g_LaraItem->pos.z);
    g_Lara.last_gun_type = g_DemoPtr[7];

    g_DemoCount += 8;

    Lara_Cheat_GetStuff();
    Random_SeedDraw(0xD371F947);
    Random_SeedControl(0xD371F947);

    g_OverlayStatus = 1;
    Camera_Initialise();
    Stats_StartTimer();

    p->text = Text_Create(0, g_PhdWinHeight - 16, GS(MISC_DEMO_MODE));
    Text_Flash(p->text, true, 20);
    Text_CentreV(p->text, true);
    Text_CentreH(p->text, true);

    return true;
}

void Demo_End(void)
{
    M_PRIV *const p = &m_Priv;
    M_RestoreConfig(p);
    M_RestoreStartInfo(p);
    Text_Remove(p->text);
    Overlay_HideGameInfo();
    Sound_StopAll();
    Music_Stop();
    Music_SetVolume(g_Config.audio.music_volume);
    p->text = NULL;
}

void Demo_Pause(void)
{
    M_PRIV *const p = &m_Priv;
    M_RestoreConfig(p);
    M_RestoreStartInfo(p);
}

void Demo_Unpause(void)
{
    M_PRIV *const p = &m_Priv;
    M_PrepareConfig(p);
    M_PrepareStartInfo(p);
    Stats_StartTimer();
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
    return Game_Control(true);
}

void Demo_StopFlashing(void)
{
    M_PRIV *const p = &m_Priv;
    Text_Flash(p->text, false, 0);
}
