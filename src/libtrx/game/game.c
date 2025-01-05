#include "game/game.h"

#include "game/random.h"

static bool m_IsPlaying = false;

void Game_SetIsPlaying(const bool is_playing)
{
    m_IsPlaying = is_playing;
    Random_FreezeDraw(!is_playing);
}

bool Game_IsPlaying(void)
{
    return m_IsPlaying;
}
