#pragma once

#include "global/types.h"

typedef enum {
    MPM_ALWAYS,
    MPM_LOOPED,
    MPM_DELAYED,
    MPM_TRACKED,
} MUSIC_PLAY_MODE;

bool __cdecl Music_Init(void);
void __cdecl Music_Shutdown(void);
void Music_Play(MUSIC_TRACK_ID track_id, MUSIC_PLAY_MODE mode);
void __cdecl Music_Stop(void);
bool __cdecl Music_PlaySynced(int16_t track_id);
double __cdecl Music_GetTimestamp(void);
bool Music_SeekTimestamp(double timestamp);
void __cdecl Music_SetVolume(int32_t volume);
MUSIC_TRACK_ID Music_GetCurrentTrack(void);
MUSIC_TRACK_ID Music_GetLastPlayedTrack(void);
MUSIC_TRACK_ID Music_GetDelayedTrack(void);
void Music_Pause(void);
void Music_Unpause(void);
int32_t __cdecl Music_GetRealTrack(int32_t track_id);

// TODO: eliminate
void __cdecl Music_Legacy_Play(int16_t track_id, bool is_looped);
