#include "decomp/stats.h"

#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/console/common.h"
#include "game/fader.h"
#include "game/input.h"
#include "game/music.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/requester.h"
#include "game/shell.h"
#include "game/text.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/log.h>

#include <stdio.h>

void ShowGymStatsText(const char *const time_str, const int32_t type)
{
    char text1[32];
    char text2[32];

    if (g_StatsRequester.ready) {
        if (!Requester_Display(&g_StatsRequester, 1, 1)) {
            g_Input = (INPUT_STATE) { 0 };
            g_InputDB = (INPUT_STATE) { 0 };
        }
        return;
    }

    g_StatsRequester.no_selector = 1;
    Requester_SetSize(&g_StatsRequester, 7, -32);
    g_StatsRequester.line_height = 18;
    g_StatsRequester.items_count = 0;
    g_StatsRequester.selected = 0;
    g_StatsRequester.line_offset = 0;
    g_StatsRequester.line_old_offset = 0;
    g_StatsRequester.pix_width = 304;
    g_StatsRequester.x_pos = 0;
    g_StatsRequester.z_pos = 0;
    g_StatsRequester.pitem_strings1 = (char *)g_ValidLevelStrings1;
    g_StatsRequester.pitem_strings2 = (char *)g_ValidLevelStrings2;
    g_StatsRequester.item_string_len = MAX_LEVEL_NAME_SIZE;

    Requester_Init(&g_StatsRequester);
    Requester_SetHeading(
        &g_StatsRequester, g_GF_GameStrings[GF_S_GAME_MISC_BEST_TIMES],
        REQ_CENTER, NULL, 0);

    int32_t i;
    for (i = 0; i < 10; i++) {
        if (!g_Assault.best_time[i]) {
            break;
        }

        sprintf(
            text1, "%2d: %s %d", i + 1, g_GF_GameStrings[GF_S_GAME_MISC_FINISH],
            g_Assault.best_finish[i]);
        const int32_t sec = g_Assault.best_time[i] / FRAMES_PER_SECOND;
        sprintf(
            text2, "%02d:%02d.%-2d", sec / 60, sec % 60,
            g_Assault.best_time[i] % FRAMES_PER_SECOND
                / (FRAMES_PER_SECOND / 10));
        Requester_AddItem(
            &g_StatsRequester, text1, REQ_ALIGN_LEFT, text2, REQ_ALIGN_RIGHT);
    }

    if (i == 0) {
        Requester_AddItem(
            &g_StatsRequester, g_GF_GameStrings[GF_S_GAME_MISC_NO_TIMES_SET],
            REQ_CENTER, NULL, REQ_CENTER);
    }

    g_StatsRequester.ready = 1;
}

int32_t AddAssaultTime(uint32_t time)
{
    ASSAULT_STATS *const stats = &g_Assault;

    int32_t insert_idx = -1;
    for (int32_t i = 0; i < MAX_ASSAULT_TIMES; i++) {
        if (stats->best_time[i] == 0 || time < stats->best_time[i]) {
            insert_idx = i;
            break;
        }
    }
    if (insert_idx == -1) {
        return false;
    }

    for (int32_t i = MAX_ASSAULT_TIMES - 1; i > insert_idx; i--) {
        stats->best_finish[i] = stats->best_finish[i - 1];
        stats->best_time[i] = stats->best_time[i - 1];
    }

    stats->finish_count++;
    stats->best_time[insert_idx] = time;
    stats->best_finish[insert_idx] = stats->finish_count;
    return true;
}
