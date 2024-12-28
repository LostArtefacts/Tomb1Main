#pragma once

#include "global/types.h"

typedef enum {
    // clang-format off
    MX_INACTIVE                = -1,
    MX_UNUSED_0                = 0, // 2.mp3
    MX_UNUSED_1                = 1, // 2.mp3
    MX_CUTSCENE_THE_GREAT_WALL = 2, // 2.mp3
    MX_UNUSED_2                = 3, // 2.mp3
    MX_CUTSCENE_OPERA_HOUSE    = 4, // 3.mp3
    MX_CUTSCENE_BROTHER_CHAN   = 5, // 4.mp3
    MX_GYM_HINT_1              = 6, // 5.mp3
    MX_GYM_HINT_2              = 7, // 6.mp3
    MX_GYM_HINT_3              = 8, // 7.mp3
    MX_GYM_HINT_4              = 9, // 8.mp3
    MX_GYM_HINT_5              = 10, // 9.mp3
    MX_GYM_HINT_6              = 11, // 10.mp3
    MX_GYM_HINT_7              = 12, // 11.mp3
    MX_GYM_HINT_8              = 13, // 12.mp3
    MX_GYM_HINT_9              = 14, // 13.mp3
    MX_GYM_HINT_10             = 15, // 14.mp3
    MX_GYM_HINT_11             = 16, // 15.mp3
    MX_GYM_HINT_12             = 17, // 16.mp3
    MX_GYM_HINT_13             = 18, // 17.mp3
    MX_GYM_HINT_14             = 19, // 18.mp3
    MX_UNUSED_3                = 20, // 18.mp3
    MX_UNUSED_4                = 21, // 18.mp3
    MX_GYM_HINT_15             = 22, // 19.mp3
    MX_GYM_HINT_16             = 23, // 20.mp3
    MX_GYM_HINT_17             = 24, // 21.mp3
    MX_GYM_HINT_18             = 25, // 22.mp3
    MX_UNUSED_5                = 26, // 23.mp3
    MX_CUTSCENE_BATH           = 27, // 23.mp3
    MX_DAGGER_PULL             = 28, // 24.mp3
    MX_GYM_HINT_20             = 29, // 25.mp3
    MX_CUTSCENE_XIAN           = 30, // 26.mp3
    MX_CAVES_AMBIENCE          = 31, // 27.mp3
    MX_SEWERS_AMBIENCE         = 32, // 28.mp3
    MX_WINDY_AMBIENCE          = 33, // 29.mp3
    MX_HEARTBEAT_AMBIENCE      = 34, // 30.mp3
    MX_SURPRISE_1              = 35, // 31.mp3
    MX_SURPRISE_2              = 36, // 32.mp3
    MX_SURPRISE_3              = 37, // 33.mp3
    MX_OOH_AAH_1               = 38, // 34.mp3
    MX_OOH_AAH_2               = 39, // 35.mp3
    MX_VENICE_VIOLINS          = 40, // 36.mp3
    MX_END_OF_LEVEL            = 41, // 37.mp3
    MX_SPOOKY_1                = 42, // 38.mp3
    MX_SPOOKY_2                = 43, // 39.mp3
    MX_SPOOKY_3                = 44, // 40.mp3
    MX_HARP_THEME              = 45, // 41.mp3
    MX_MYSTERY_1               = 46, // 42.mp3
    MX_SECRET                  = 47, // 43.mp3
    MX_AMBUSH_1                = 48, // 44.mp3
    MX_AMBUSH_2                = 49, // 45.mp3
    MX_AMBUSH_3                = 50, // 46.mp3
    MX_AMBUSH_4                = 51, // 47.mp3
    MX_SKIDOO_THEME            = 52, // 48.mp3
    MX_BATTLE_THEME            = 53, // 49.mp3
    MX_MYSTERY_2               = 54, // 50.mp3
    MX_MYSTERY_3               = 55, // 51.mp3
    MX_MYSTERY_4               = 56, // 52.mp3
    MX_MYSTERY_5               = 57, // 53.mp3
    MX_RIG_AMBIENCE            = 58, // 54.mp3
    MX_TOMB_AMBIENCE           = 59, // 55.mp3
    MX_OOH_AAH_3               = 60, // 56.mp3
    MX_REVEAL_1                = 61, // 57.mp3
    MX_CUTSCENE_RIG            = 62, // 58.mp3
    MX_REVEAL_2                = 63, // 59.mp3
    MX_TITLE_THEME             = 64, // 60.mp3
    MX_UNUSED_6                = 65, // 61.mp3
    // clang-format on
} MUSIC_TRACK_ID;

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

// TODO: eliminate
void Music_Legacy_Play(int16_t track_id, bool is_looped);
