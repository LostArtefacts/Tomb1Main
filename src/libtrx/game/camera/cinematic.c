#include "game/camera/types.h"
#include "game/camera/vars.h"
#include "game/game_buf.h"

static CINE_FRAME *m_CineFrames = nullptr;
static CINE_DATA m_CineData = {};

void Camera_InitialiseCineFrames(const int32_t num_frames)
{
    m_CineData.frame_count = num_frames;
    m_CineData.frame_idx = 0;
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
    return Camera_GetCineFrame(m_CineData.frame_idx);
}

CINE_DATA *Camera_GetCineData(void)
{
    return &m_CineData;
}

void Camera_InvokeCinematic(
    const ITEM *const item, const int32_t frame_idx, const int16_t extra_y_rot)
{
    g_Camera.type = CAM_CINEMATIC;
    m_CineData.frame_idx = frame_idx;
    m_CineData.position.pos = item->pos;
    m_CineData.position.rot = item->rot;
    m_CineData.position.rot.y += extra_y_rot;
}
