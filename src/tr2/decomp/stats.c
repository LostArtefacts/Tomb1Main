#include "decomp/stats.h"

#include "global/vars.h"

#include <libtrx/log.h>

#include <stdio.h>

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
