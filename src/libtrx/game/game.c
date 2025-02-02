#include "game/game.h"

#include "game/game_flow.h"
#include "game/lara/common.h"
#include "game/random.h"

static bool m_IsPlaying = false;
static const GF_LEVEL *m_CurrentLevel = nullptr;

void Game_SetIsPlaying(const bool is_playing)
{
    m_IsPlaying = is_playing;
    Random_FreezeDraw(!is_playing);
}

bool Game_IsPlaying(void)
{
    return m_IsPlaying;
}

const GF_LEVEL *Game_GetCurrentLevel(void)
{
    return m_CurrentLevel;
}

void Game_SetCurrentLevel(const GF_LEVEL *const level)
{
    m_CurrentLevel = level;
}

bool Game_IsInGym(void)
{
    const GF_LEVEL *const current_level = GF_GetCurrentLevel();
    return current_level != nullptr && current_level->type == GFL_GYM;
}

bool Game_IsPlayable(void)
{
    const GF_LEVEL *const current_level = GF_GetCurrentLevel();
    if (current_level == nullptr || current_level->type == GFL_TITLE
        || current_level->type == GFL_DEMO
        || current_level->type == GFL_CUTSCENE) {
        return false;
    }

    if (!Object_Get(O_LARA)->loaded || Lara_GetItem() == nullptr) {
        return false;
    }

    return true;
}
