#include "game/camera/types.h"
#include "game/camera/vars.h"
#include "game/game_buf.h"

static CINE_FRAME *m_CineFrames = NULL;

void Camera_InitialiseCineFrames(const int32_t num_frames)
{
    g_CineData.frame_count = num_frames;
    g_CineData.frame_idx = 0;
    m_CineFrames = num_frames == 0
        ? nullptr
        : GameBuf_Alloc(num_frames * sizeof(CINE_FRAME), GBUF_CINEMATIC_FRAMES);
}

CINE_FRAME *Camera_GetCineFrame(const int32_t frame_idx)
{
    if (m_CineFrames == nullptr) {
        return nullptr;
    }
    return &m_CineFrames[frame_idx];
}

CINE_FRAME *Camera_GetCurrentCineFrame(void)
{
    return Camera_GetCineFrame(g_CineData.frame_idx);
}
