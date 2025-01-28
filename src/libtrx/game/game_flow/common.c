#include "game/game_flow/common.h"

#include "game/game_flow/vars.h"
#include "memory.h"

static const GAME_FLOW_LEVEL *m_CurrentLevel = NULL;
static GAME_FLOW_COMMAND m_OverrideCommand = { .action = GF_NOOP };

static void M_FreeSequence(GAME_FLOW_SEQUENCE *sequence);
static void M_FreeInjections(INJECTION_DATA *injections);
static void M_FreeLevel(GAME_FLOW_LEVEL *level);
static void M_FreeLevels(GAME_FLOW_LEVEL **levels, int32_t *level_count);
static void M_FreeFMVs(GAME_FLOW *gf);

static void M_FreeSequence(GAME_FLOW_SEQUENCE *const sequence)
{
    Memory_Free(sequence->events);
}

static void M_FreeInjections(INJECTION_DATA *const injections)
{
    for (int32_t i = 0; i < injections->count; i++) {
        Memory_FreePointer(&injections->data_paths[i]);
    }
    Memory_FreePointer(&injections->data_paths);
}

static void M_FreeLevel(GAME_FLOW_LEVEL *const level)
{
    Memory_FreePointer(&level->path);
    Memory_FreePointer(&level->title);
    M_FreeInjections(&level->injections);
    M_FreeSequence(&level->sequence);

#if TR_VERSION == 1
    if (level->item_drops.count > 0) {
        for (int32_t i = 0; i < level->item_drops.count; i++) {
            Memory_FreePointer(&level->item_drops.data[i].object_ids);
        }
        Memory_FreePointer(&level->item_drops.data);
    }
#endif
}

static void M_FreeLevels(GAME_FLOW_LEVEL **levels, int32_t *const level_count)
{
    if (levels != NULL) {
        for (int32_t i = 0; i < *level_count; i++) {
            M_FreeLevel(&(*levels)[i]);
        }
        Memory_FreePointer(levels);
    }
    *level_count = 0;
}

static void M_FreeFMVs(GAME_FLOW *const gf)
{
    for (int32_t i = 0; i < gf->fmv_count; i++) {
        Memory_FreePointer(&gf->fmvs[i].path);
    }
    Memory_FreePointer(&gf->fmvs);
    gf->fmv_count = 0;
}

void GF_Shutdown(void)
{
    GAME_FLOW *const gf = &g_GameFlow;
    M_FreeInjections(&gf->injections);

    M_FreeLevels(&gf->levels, &gf->level_count);
    M_FreeLevels(&gf->demos, &gf->demo_count);
    M_FreeLevels(&gf->cutscenes, &gf->cutscene_count);
    M_FreeFMVs(gf);

    if (gf->title_level != NULL) {
        M_FreeLevel(gf->title_level);
        Memory_FreePointer(&gf->title_level);
    }

#if TR_VERSION == 1
    Memory_FreePointer(&gf->main_menu_background_path);
    Memory_FreePointer(&gf->savegame_fmt_legacy);
    Memory_FreePointer(&gf->savegame_fmt_bson);
#endif
}

bool GF_IsGymEnabled(void)
{
    return GF_GetGymLevelNum() != -1;
}

void GF_OverrideCommand(const GAME_FLOW_COMMAND command)
{
    m_OverrideCommand = command;
}

GAME_FLOW_COMMAND GF_GetOverrideCommand(void)
{
    return m_OverrideCommand;
}

const GAME_FLOW_LEVEL *GF_GetCurrentLevel(void)
{
    return m_CurrentLevel;
}

void GF_SetCurrentLevel(const GAME_FLOW_LEVEL *const level)
{
    m_CurrentLevel = level;
}
