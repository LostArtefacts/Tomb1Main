#include "game/camera/vars.h"
#include "game/game_buf.h"

static int32_t m_FixedObjectCount = 0;

void Camera_InitialiseFixedObjects(const int32_t num_objects)
{
    m_FixedObjectCount = num_objects;
    g_Camera.fixed = num_objects == 0
        ? nullptr
        : GameBuf_Alloc(num_objects * sizeof(OBJECT_VECTOR), GBUF_CAMERAS);
}

int32_t Camera_GetFixedObjectCount(void)
{
    return m_FixedObjectCount;
}
