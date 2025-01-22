#include "game/stats.h"

#include "game/clock.h"
#include "game/game_flow.h"
#include "global/vars.h"

#define USE_REAL_CLOCK 0

#if USE_REAL_CLOCK
static CLOCK_TIMER m_StartCounter = { .type = CLOCK_TYPE_REAL };
static int32_t m_StartTimer = 0;

void Stats_StartTimer(void)
{
    ClockTimer_Sync(&m_StartCounter);
    m_StartTimer = g_SaveGame.current_stats.timer;
}

void Stats_UpdateTimer(void)
{
    const double elapsed = ClockTimer_PeekElapsed(&m_StartCounter) * LOGIC_FPS;
    g_SaveGame.current_stats.timer = m_StartTimer + elapsed;
}
#else
void Stats_StartTimer(void)
{
}

void Stats_UpdateTimer(void)
{
    g_SaveGame.current_stats.timer++;
}
#endif

FINAL_STATS Stats_ComputeFinalStats(void)
{
    FINAL_STATS result = {};

    const int32_t total_levels = GF_GetLevelCount();
    for (int32_t i = LV_FIRST; i < total_levels; i++) {
        result.timer += g_SaveGame.start[i].stats.timer;
        result.ammo_used += g_SaveGame.start[i].stats.ammo_used;
        result.ammo_hits += g_SaveGame.start[i].stats.ammo_hits;
        result.kills += g_SaveGame.start[i].stats.kills;
        result.distance += g_SaveGame.start[i].stats.distance;
        result.medipacks += g_SaveGame.start[i].stats.medipacks;

        // TODO: #170, consult GFE_NUM_SECRETS rather than hardcoding this
        if (i < total_levels - 2) {
            for (int32_t j = 0; j < 3; j++) {
                if (g_SaveGame.start[i].stats.secret_flags & (1 << j)) {
                    result.found_secrets++;
                }
                result.total_secrets++;
            }
        }
    }

    return result;
}
