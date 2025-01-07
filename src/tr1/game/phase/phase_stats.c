#include "game/phase/phase_stats.h"

#include "game/game.h"
#include "game/game_string.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/shell.h"
#include "game/stats.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/debug.h>
#include <libtrx/memory.h>

#define MAX_TEXTSTRINGS 10

typedef enum {
    STATE_FADE_IN,
    STATE_DISPLAY,
    STATE_FADE_OUT,
} STATE;

typedef struct {
    PHASE_STATS_ARGS args;
    STATE state;
    TEXTSTRING *texts[MAX_TEXTSTRINGS];
    FADER back_fader;
    FADER top_fader;
} M_PRIV;

static bool M_IsFading(M_PRIV *p);
static void M_FadeIn(M_PRIV *p);
static void M_FadeOut(M_PRIV *p);
static void M_CreateTexts(M_PRIV *p, int32_t level_num);
static void M_CreateTextsTotal(M_PRIV *p, GAME_FLOW_LEVEL_TYPE level_type);

static PHASE_CONTROL M_Start(PHASE *phase);
static void M_End(PHASE *phase);
static PHASE_CONTROL M_Control(PHASE *phase, int32_t num_frames);
static void M_Draw(PHASE *phase);

static bool M_IsFading(M_PRIV *const p)
{
    return Fader_IsActive(&p->top_fader) || Fader_IsActive(&p->back_fader);
}

static void M_FadeIn(M_PRIV *const p)
{
    if (p->args.background_path != NULL) {
        Fader_Init(&p->top_fader, FADER_BLACK, FADER_TRANSPARENT, 1.0);
    } else {
        Fader_Init(&p->back_fader, FADER_TRANSPARENT, FADER_SEMI_BLACK, 0.5);
    }
}

static void M_FadeOut(M_PRIV *const p)
{
    Fader_Init(&p->top_fader, FADER_ANY, FADER_BLACK, 0.5);
    p->state = STATE_FADE_OUT;
}

static void M_CreateTexts(M_PRIV *const p, const int32_t level_num)
{
    char buf[100];
    char time_str[100];

    const GAME_STATS *stats = &g_GameInfo.current[level_num].stats;

    Overlay_HideGameInfo();

    int y = -50;
    const int row_height = 30;

    TEXTSTRING **cur_txt = &p->texts[0];

    // heading
    sprintf(buf, "%s", g_GameFlow.levels[level_num].level_title);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;

    // kills
    sprintf(
        buf,
        g_Config.gameplay.enable_detailed_stats ? GS(STATS_KILLS_DETAIL_FMT)
                                                : GS(STATS_KILLS_BASIC_FMT),
        stats->kill_count, stats->max_kill_count);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;

    // pickups
    sprintf(
        buf,
        g_Config.gameplay.enable_detailed_stats ? GS(STATS_PICKUPS_DETAIL_FMT)
                                                : GS(STATS_PICKUPS_BASIC_FMT),
        stats->pickup_count, stats->max_pickup_count);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;

    // secrets
    int secret_count = 0;
    int16_t secret_flags = stats->secret_flags;
    for (int i = 0; i < MAX_SECRETS; i++) {
        if (secret_flags & 1) {
            secret_count++;
        }
        secret_flags >>= 1;
    }
    sprintf(buf, GS(STATS_SECRETS_FMT), secret_count, stats->max_secret_count);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;

    // deaths
    if (g_Config.gameplay.enable_deaths_counter
        && g_GameInfo.death_counter_supported) {
        sprintf(buf, GS(STATS_DEATHS_FMT), stats->death_count);
        *cur_txt = Text_Create(0, y, buf);
        Text_CentreH(*cur_txt, 1);
        Text_CentreV(*cur_txt, 1);
        cur_txt++;
        y += row_height;
    }

    // time taken
    int seconds = stats->timer / LOGIC_FPS;
    int hours = seconds / 3600;
    int minutes = (seconds / 60) % 60;
    seconds %= 60;
    if (hours) {
        sprintf(
            time_str, "%d:%d%d:%d%d", hours, minutes / 10, minutes % 10,
            seconds / 10, seconds % 10);
    } else {
        sprintf(time_str, "%d:%d%d", minutes, seconds / 10, seconds % 10);
    }
    sprintf(buf, GS(STATS_TIME_TAKEN_FMT), time_str);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;
}

static void M_CreateTextsTotal(
    M_PRIV *const p, const GAME_FLOW_LEVEL_TYPE level_type)
{
    TOTAL_STATS stats;
    Stats_ComputeTotal(level_type, &stats);

    char buf[100];
    char time_str[100];
    TEXTSTRING **cur_txt = &p->texts[0];

    int top_y = 55;
    int y = 55;
    const int row_width = 220;
    const int row_height = 20;
    int16_t border = 4;

    // reserve space for heading
    y += row_height + border * 2;

    // kills
    sprintf(
        buf,
        g_Config.gameplay.enable_detailed_stats ? GS(STATS_KILLS_DETAIL_FMT)
                                                : GS(STATS_KILLS_BASIC_FMT),
        stats.player_kill_count, stats.total_kill_count);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;

    // pickups
    sprintf(
        buf,
        g_Config.gameplay.enable_detailed_stats ? GS(STATS_PICKUPS_DETAIL_FMT)
                                                : GS(STATS_PICKUPS_BASIC_FMT),
        stats.player_pickup_count, stats.total_pickup_count);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;

    // secrets
    sprintf(
        buf, GS(STATS_SECRETS_FMT), stats.player_secret_count,
        stats.total_secret_count);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;

    // deaths
    if (g_Config.gameplay.enable_deaths_counter
        && g_GameInfo.death_counter_supported) {
        sprintf(buf, GS(STATS_DEATHS_FMT), stats.death_count);
        *cur_txt = Text_Create(0, y, buf);
        Text_CentreH(*cur_txt, 1);
        Text_CentreV(*cur_txt, 1);
        cur_txt++;
        y += row_height;
    }

    // time taken
    int seconds = stats.timer / LOGIC_FPS;
    int hours = seconds / 3600;
    int minutes = (seconds / 60) % 60;
    seconds %= 60;
    if (hours) {
        sprintf(
            time_str, "%d:%d%d:%d%d", hours, minutes / 10, minutes % 10,
            seconds / 10, seconds % 10);
    } else {
        sprintf(time_str, "%d:%d%d", minutes, seconds / 10, seconds % 10);
    }
    sprintf(buf, GS(STATS_TIME_TAKEN_FMT), time_str);
    *cur_txt = Text_Create(0, y, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    cur_txt++;
    y += row_height;

    // border
    int16_t height = y + border * 2 - top_y;
    *cur_txt = Text_Create(0, top_y, " ");
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    Text_AddBackground(*cur_txt, row_width, height, 0, 0, TS_BACKGROUND);
    Text_AddOutline(*cur_txt, TS_BACKGROUND);
    cur_txt++;

    // heading
    sprintf(
        buf, "%s",
        level_type == GFL_BONUS ? GS(STATS_BONUS_STATISTICS)
                                : GS(STATS_FINAL_STATISTICS));
    *cur_txt = Text_Create(0, top_y + 2, buf);
    Text_CentreH(*cur_txt, 1);
    Text_CentreV(*cur_txt, 1);
    Text_AddBackground(*cur_txt, row_width - 4, 0, 0, 0, TS_HEADING);
    Text_AddOutline(*cur_txt, TS_HEADING);
    cur_txt++;
}

static PHASE_CONTROL M_Start(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;

    if (p->args.background_path != NULL) {
        Output_LoadBackgroundFromFile(p->args.background_path);
    } else {
        Output_UnloadBackground();
    }

    if (g_CurrentLevel == g_GameFlow.gym_level_num) {
        M_FadeOut(p);
    } else {
        M_FadeIn(p);

        if (p->args.show_final_stats) {
            ASSERT(p->args.level_type >= GFL_NORMAL);
            M_CreateTextsTotal(p, p->args.level_type);
        } else {
            M_CreateTexts(
                p,
                p->args.level_num != -1 ? p->args.level_num : g_CurrentLevel);
        }
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Music_Stop();

    for (int32_t i = 0; i < MAX_TEXTSTRINGS; i++) {
        TEXTSTRING *cur_txt = p->texts[i];
        if (cur_txt != NULL) {
            Text_Remove(cur_txt);
        }
    }
}

static PHASE_CONTROL M_Control(PHASE *const phase, int32_t num_frames)
{
    M_PRIV *const p = phase->priv;
    Input_Update();
    Shell_ProcessInput();

    switch (p->state) {
    case STATE_FADE_IN:
        if (!M_IsFading(p)) {
            p->state = STATE_DISPLAY;
        } else if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
            M_FadeOut(p);
        }
        break;

    case STATE_DISPLAY:
        if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
            M_FadeOut(p);
        }
        break;

    case STATE_FADE_OUT:
        M_FadeOut(p);
        if (g_InputDB.menu_confirm || g_InputDB.menu_back || !M_IsFading(p)) {
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_NOOP },
            };
        }
        break;
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_Draw(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    if (!p->args.show_final_stats) {
        Interpolation_Disable();
        Game_DrawScene(false);
        Interpolation_Enable();
        Output_DrawBlackRectangle(Fader_GetCurrentValue(&p->back_fader));
    }
    Text_Draw();
    Output_DrawBlackRectangle(Fader_GetCurrentValue(&p->top_fader));
}

PHASE *Phase_Stats_Create(const PHASE_STATS_ARGS args)
{
    PHASE *const phase = Memory_Alloc(sizeof(PHASE));
    M_PRIV *const p = Memory_Alloc(sizeof(M_PRIV));
    p->args = args;
    p->state = STATE_FADE_IN;
    phase->priv = p;
    phase->start = M_Start;
    phase->end = M_End;
    phase->control = M_Control;
    phase->draw = M_Draw;
    return phase;
}

void Phase_Stats_Destroy(PHASE *const phase)
{
    M_PRIV *const p = phase->priv;
    Memory_Free(p);
    Memory_Free(phase);
}
