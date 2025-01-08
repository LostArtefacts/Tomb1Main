#include "game/sound.h"

#include "engine/audio.h"

void Sound_PauseAll(void)
{
    Audio_Sample_PauseAll();
}

void Sound_UnpauseAll(void)
{
    Audio_Sample_UnpauseAll();
}
