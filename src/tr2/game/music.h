#pragma once

#include "global/types.h"

#include <libtrx/game/music.h>

typedef enum {
    MPM_ALWAYS,
    MPM_LOOPED,
    MPM_DELAYED,
    MPM_TRACKED,
} MUSIC_PLAY_MODE;

bool Music_Init(void);
void Music_Shutdown(void);
void Music_Play(MUSIC_TRACK_ID track_id, MUSIC_PLAY_MODE mode);
void Music_Stop(void);
bool Music_PlaySynced(int16_t track_id);
double Music_GetTimestamp(void);
bool Music_SeekTimestamp(double timestamp);
void Music_SetVolume(int32_t volume);
MUSIC_TRACK_ID Music_GetCurrentPlayingTrack(void);
MUSIC_TRACK_ID Music_GetCurrentLoopedTrack(void);
MUSIC_TRACK_ID Music_GetLastPlayedTrack(void);
MUSIC_TRACK_ID Music_GetDelayedTrack(void);
void Music_Pause(void);
void Music_Unpause(void);
int32_t Music_GetRealTrack(int32_t track_id);
