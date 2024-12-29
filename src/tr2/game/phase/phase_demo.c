#include "game/phase/phase_demo.h"

#include "config.h"
#include "decomp/decomp.h"
#include "game/camera.h"
#include "game/fader.h"
#include "game/game.h"
#include "game/input.h"
#include "game/inventory_ring.h"
#include "game/items.h"
#include "game/lara/cheat.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/phase/priv.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/log.h>
#include <libtrx/memory.h>

typedef struct {
    bool exiting;
    int32_t level_num;
    TEXTSTRING *text;
    FADER exit_fader;

    struct {
        bool bonus_flag;
    } old_config;
    START_INFO old_start;
} M_PRIV;

static void M_PrepareConfig(M_PRIV *p);
static void M_RestoreConfig(M_PRIV *p);
static void M_PrepareStartInfo(M_PRIV *p);
static void M_RestoreStartInfo(M_PRIV *p);
static bool M_LoadLevel(M_PRIV *p);
static void M_PrepareGame(void);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t n_frames);
static void M_Draw(PHASE *phase);

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
    START_INFO *const start = &g_SaveGame.start[p->level_num];
    p->old_start = *start;
    start->available = 1;
    start->has_pistols = 1;
    start->pistol_ammo = 1000;
    start->gun_status = LGS_ARMLESS;
    start->gun_type = LGT_PISTOLS;
}

static void M_RestoreStartInfo(M_PRIV *const p)
{
    g_SaveGame.start[p->level_num] = p->old_start;
}

static bool M_LoadLevel(M_PRIV *const p)
{
    M_PrepareStartInfo(p);
    Random_SeedDraw(0xD371F947);
    Random_SeedControl(0xD371F947);

    g_IsTitleLoaded = false;
    if (!Level_Initialise(p->level_num, GFL_DEMO)) {
        return false;
    }

    g_LevelComplete = false;
    if (!g_IsDemoLoaded) {
        LOG_ERROR(
            "Level '%s' has no demo data", g_GF_LevelFileNames[p->level_num]);
        return false;
    }
    return true;
}

static void M_PrepareGame(void)
{
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

    g_Inv_DemoMode = true;
    g_OverlayStatus = 1;
    Camera_Initialise();
    g_NoInputCounter = 0;
    Stats_StartTimer();
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (p->level_num < 0) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .dir = GFD_EXIT_TO_TITLE,
        };
    }

    M_PrepareConfig(p);
    if (!M_LoadLevel(p)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .dir = GFD_EXIT_TO_TITLE,
        };
    }
    M_PrepareGame();

    p->text =
        Text_Create(0, g_PhdWinHeight - 16, g_GF_PCStrings[GF_S_PC_DEMO_MODE]);
    Text_Flash(p->text, true, 20);
    Text_CentreV(p->text, true);
    Text_CentreH(p->text, true);

    g_OldInputDB = g_Input;

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;

    Text_Remove(p->text);

    Overlay_HideGameInfo();
    Sound_StopAllSamples();
    Music_Stop();
    Music_SetVolume(g_Config.audio.music_volume);
    g_Inv_DemoMode = false;
    M_RestoreStartInfo(p);
    M_RestoreConfig(p);
}

static PHASE_CONTROL M_Control(PHASE *const phase, const int32_t num_frames)
{
    M_PRIV *const p = phase->priv;
    if (g_IsGameToExit && !p->exiting) {
        p->exiting = true;
        Fader_InitAnyToBlack(&p->exit_fader, FRAMES_PER_SECOND / 3);
    } else if (p->exiting && !Fader_IsActive(&p->exit_fader)) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .dir = GFD_EXIT_GAME,
        };
    } else {
        Fader_Control(&p->exit_fader);

        const GAME_FLOW_DIR dir = Game_Control(num_frames, true);
        if (dir != (GAME_FLOW_DIR)-1) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .dir = dir,
            };
        }
    }
    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Game_Draw();
    Output_DrawBlackRectangle(Fader_GetCurrentValue(&p->exit_fader));
}

PHASE *Phase_Demo_Create(const int32_t level_num)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->level_num = level_num;
    phase->priv = p;
    phase->start = M_Start;
    phase->end = M_End;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Demo_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
