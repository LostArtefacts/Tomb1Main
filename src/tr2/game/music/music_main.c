#include "game/music.h"

#include "game/music/music_backend.h"
#include "game/music/music_backend_cdaudio.h"
#include "game/music/music_backend_files.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/engine/audio.h>
#include <libtrx/log.h>

static MUSIC_TRACK_ID m_TrackCurrent = MX_INACTIVE;
static MUSIC_TRACK_ID m_TrackLastPlayed = MX_INACTIVE;
static MUSIC_TRACK_ID m_TrackDelayed = MX_INACTIVE;
static MUSIC_TRACK_ID m_TrackLooped = MX_INACTIVE;

static float m_MusicVolume = 0.0f;
static int32_t m_AudioStreamID = -1;
static const MUSIC_BACKEND *m_Backend = NULL;

static const MUSIC_BACKEND *M_FindBackend(void);
static void M_StopActiveStream(void);
static void M_StreamFinished(int32_t stream_id, void *user_data);

static const MUSIC_BACKEND *M_FindBackend(void)
{
    MUSIC_BACKEND *all_backends[] = {
        Music_Backend_Files_Factory("music"),
        Music_Backend_CDAudio_Factory("audio/cdaudio.wav"),
        Music_Backend_CDAudio_Factory("audio/cdaudio.mp3"),
        NULL,
    };

    MUSIC_BACKEND **backend_ptr = all_backends;
    while (true) {
        MUSIC_BACKEND *backend = *backend_ptr;
        if (backend == NULL) {
            break;
        }
        if (backend->init(backend)) {
            return backend;
        }
        backend_ptr++;
    }
    return NULL;
}

static void M_StopActiveStream(void)
{
    if (m_AudioStreamID < 0) {
        return;
    }

    // We are only interested in calling M_StreamFinished if a stream
    // finished by itself. In cases where we end the streams early by hand,
    // we clear the finish callback in order to avoid resuming the BGM playback
    // just after we stop it.
    Audio_Stream_SetFinishCallback(m_AudioStreamID, NULL, NULL);
    Audio_Stream_Close(m_AudioStreamID);
}

static void M_StreamFinished(const int32_t stream_id, void *const user_data)
{
    // When a stream finishes, play the remembered background BGM.
    if (stream_id == m_AudioStreamID) {
        m_TrackCurrent = MX_INACTIVE;
        m_AudioStreamID = -1;
        if (m_TrackLooped >= 0) {
            Music_Play(m_TrackLooped, MPM_LOOPED);
        }
    }
}

bool __cdecl Music_Init(void)
{
    bool result = false;

    m_Backend = M_FindBackend();
    if (m_Backend == NULL) {
        LOG_ERROR("No music backend is available");
        goto finish;
    }

    LOG_ERROR("Chosen music backend: %s", m_Backend->describe(m_Backend));
    result = true;
    Music_SetVolume(25 * g_OptionMusicVolume + 5);

finish:
    m_TrackCurrent = MX_INACTIVE;
    m_TrackLastPlayed = MX_INACTIVE;
    m_TrackDelayed = MX_INACTIVE;
    m_TrackLooped = MX_INACTIVE;
    return result;
}

void __cdecl Music_Shutdown(void)
{
    if (m_AudioStreamID < 0) {
        return;
    }

    // We are only interested in calling M_StreamFinished if a stream
    // finished by itself. In cases where we end the streams early by hand,
    // we clear the finish callback in order to avoid resuming the BGM playback
    // just after we stop it.
    Audio_Stream_SetFinishCallback(m_AudioStreamID, NULL, NULL);
    Audio_Stream_Close(m_AudioStreamID);
}

void __cdecl Music_Legacy_Play(int16_t track_id, bool is_looped)
{
    Music_Play(track_id, is_looped ? MPM_LOOPED : MPM_ALWAYS);
}

void Music_Play(const MUSIC_TRACK_ID track_id, const MUSIC_PLAY_MODE mode)
{
    if (track_id == m_TrackCurrent && mode != MPM_ALWAYS) {
        return;
    }

    if (mode == MPM_TRACKED && track_id == m_TrackLastPlayed) {
        return;
    }

    if (mode == MPM_DELAYED) {
        m_TrackDelayed = track_id;
        return;
    }

    M_StopActiveStream();

    if (g_OptionMusicVolume == 0) {
        LOG_DEBUG("Not playing track %d because the game is silent", track_id);
        goto finish;
    }

    if (m_Backend == NULL) {
        LOG_DEBUG(
            "Not playing track %d because no backend is available", track_id);
        goto finish;
    }

    const int32_t real_track_id = Music_GetRealTrack(track_id);
    LOG_DEBUG(
        "Playing track %d (real: %d), mode: %d", track_id, real_track_id, mode);

    m_AudioStreamID = m_Backend->play(m_Backend, real_track_id);
    if (m_AudioStreamID < 0) {
        LOG_ERROR("Failed to create music stream for track %d", track_id);
        goto finish;
    }

    Audio_Stream_SetIsLooped(m_AudioStreamID, mode == MPM_LOOPED);
    Audio_Stream_SetVolume(m_AudioStreamID, m_MusicVolume);
    Audio_Stream_SetFinishCallback(m_AudioStreamID, M_StreamFinished, NULL);

finish:
    m_TrackDelayed = MX_INACTIVE;
    if (mode == MPM_LOOPED) {
        m_TrackLooped = track_id;
    } else {
        m_TrackCurrent = track_id;
        m_TrackLastPlayed = track_id;
    }
}

void __cdecl Music_Stop(void)
{
    if (m_AudioStreamID < 0) {
        return;
    }
    m_TrackCurrent = MX_INACTIVE;
    m_TrackLastPlayed = MX_INACTIVE;
    m_TrackDelayed = MX_INACTIVE;
    m_TrackLooped = MX_INACTIVE;
    M_StopActiveStream();
}

bool __cdecl Music_PlaySynced(int16_t track_id)
{
    Music_Play(track_id, false);
    return true;
}

double __cdecl Music_GetTimestamp(void)
{
    if (m_AudioStreamID < 0) {
        return -1.0;
    }
    return Audio_Stream_GetTimestamp(m_AudioStreamID);
}

void __cdecl Music_SetVolume(int32_t volume)
{
    m_MusicVolume = volume ? volume / 255.0f : 0.0f;
    if (m_AudioStreamID >= 0) {
        Audio_Stream_SetVolume(m_AudioStreamID, m_MusicVolume);
    }
}

MUSIC_TRACK_ID Music_GetCurrentTrack(void)
{
    return m_TrackCurrent != MX_INACTIVE ? m_TrackCurrent : m_TrackLooped;
}

MUSIC_TRACK_ID Music_GetLastPlayedTrack(void)
{
    return m_TrackLastPlayed;
}

MUSIC_TRACK_ID Music_GetDelayedTrack(void)
{
    return m_TrackDelayed;
}

void Music_Pause(void)
{
    if (m_AudioStreamID < 0) {
        return;
    }
    Audio_Stream_Pause(m_AudioStreamID);
}

void Music_Unpause(void)
{
    if (m_AudioStreamID < 0) {
        return;
    }
    Audio_Stream_Unpause(m_AudioStreamID);
}

int32_t __cdecl Music_GetRealTrack(const int32_t track_id)
{
    const int8_t skipped_track_ids[] = { 2, 19, 20, 26, -1 };
    int32_t idx = 0;
    int32_t ret_track_id = 2;

    for (int32_t i = 2; i < track_id; i++) {
        if ((skipped_track_ids[idx] >= 0) && (i == skipped_track_ids[idx])) {
            idx++;
        } else {
            ret_track_id++;
        }
    }
    return ret_track_id;
}
