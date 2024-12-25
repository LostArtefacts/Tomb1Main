#include "game/stats.h"

#include "game/clock.h"
#include "global/vars.h"

static double m_StartCounter = 0.0;
static int32_t m_StartTimer = 0;

void Stats_StartTimer(void)
{
    m_StartCounter = Clock_GetHighPrecisionCounter();
    m_StartTimer = g_SaveGame.statistics.timer;
}

void Stats_UpdateTimer(void)
{
    const double elapsed =
        (Clock_GetHighPrecisionCounter() - m_StartCounter) * LOGIC_FPS / 1000.0;
    g_SaveGame.statistics.timer = m_StartTimer + elapsed;
}

FINAL_STATS Stats_ComputeFinalStats(void)
{
    FINAL_STATS result = {};

    const int32_t total_levels = g_GameFlow.num_levels - g_GameFlow.num_demos;
    for (int32_t i = LV_FIRST; i < total_levels; i++) {
        result.timer += g_SaveGame.start[i].statistics.timer;
        result.ammo_used += g_SaveGame.start[i].statistics.shots;
        result.ammo_hits += g_SaveGame.start[i].statistics.hits;
        result.kills += g_SaveGame.start[i].statistics.kills;
        result.distance += g_SaveGame.start[i].statistics.distance;
        result.medipacks += g_SaveGame.start[i].statistics.medipacks;

        // TODO: #170, consult GFE_NUM_SECRETS rather than hardcoding this
        if (i < total_levels - 2) {
            for (int32_t j = 0; j < 3; j++) {
                if (g_SaveGame.start[i].statistics.secrets & (1 << j)) {
                    result.found_secrets++;
                }
                result.total_secrets++;
            }
        }
    }

    return result;
}
