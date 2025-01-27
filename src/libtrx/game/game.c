#include "game/game.h"

#include "game/random.h"

#include <stddef.h>

static bool m_IsPlaying = false;
static const GAME_FLOW_LEVEL *m_CurrentLevel = NULL;

void Game_SetIsPlaying(const bool is_playing)
{
    m_IsPlaying = is_playing;
    Random_FreezeDraw(!is_playing);
}

bool Game_IsPlaying(void)
{
    return m_IsPlaying;
}

const GAME_FLOW_LEVEL *Game_GetCurrentLevel(void)
{
    return m_CurrentLevel;
}

void Game_SetCurrentLevel(const GAME_FLOW_LEVEL *const level)
{
    m_CurrentLevel = level;
}
