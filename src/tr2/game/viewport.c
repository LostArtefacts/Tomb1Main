#include "game/viewport.h"

#include "config.h"
#include "game/math.h"
#include "game/output.h"
#include "global/const.h"
#include "global/vars.h"

static VIEWPORT m_Viewport = { 0 };

static void M_Apply(void);

static void M_Apply(void)
{
    // TODO: remove most of these variables if possible
    g_PhdWinMinX = m_Viewport.x;
    g_PhdWinMinY = m_Viewport.y;
    g_PhdWinMaxX = m_Viewport.width - 1;
    g_PhdWinMaxY = m_Viewport.height - 1;
    g_PhdWinWidth = m_Viewport.width;
    g_PhdWinHeight = m_Viewport.height;

    g_PhdWinCenterX = m_Viewport.width / 2;
    g_PhdWinCenterY = m_Viewport.height / 2;
    g_FltWinCenterX = m_Viewport.width / 2;
    g_FltWinCenterY = m_Viewport.height / 2;

    g_PhdWinLeft = 0;
    g_PhdWinTop = 0;
    g_PhdWinRight = g_PhdWinMaxX;
    g_PhdWinBottom = g_PhdWinMaxY;

    g_PhdWinRect.left = g_PhdWinMinX;
    g_PhdWinRect.bottom = g_PhdWinMinY + g_PhdWinHeight;
    g_PhdWinRect.top = g_PhdWinMinY;
    g_PhdWinRect.right = g_PhdWinMinX + g_PhdWinWidth;

    g_PhdNearZ = m_Viewport.near_z << W2V_SHIFT;
    g_PhdFarZ = m_Viewport.far_z << W2V_SHIFT;

    g_FltNearZ = g_PhdNearZ;
    g_FltFarZ = g_PhdFarZ;

    const double res_z =
        0.99 * g_FltNearZ * g_FltFarZ / (g_FltFarZ - g_FltNearZ);
    g_FltResZ = res_z;
    g_FltResZORhw = res_z / g_RhwFactor;
    g_FltResZBuf = 0.005 + res_z / g_FltNearZ;
    g_FltRhwONearZ = g_RhwFactor / g_FltNearZ;
    g_FltPerspONearZ = g_FltPersp / g_FltNearZ;
    g_PhdViewDistance = m_Viewport.far_z;

    Viewport_AlterFOV(m_Viewport.view_angle);

    g_PhdScreenWidth = m_Viewport.screen_width;
    g_PhdScreenHeight = m_Viewport.screen_height;
}

void Viewport_Init(
    int16_t x, int16_t y, int32_t width, int32_t height, int32_t near_z,
    int32_t far_z, int16_t view_angle, int32_t screen_width,
    int32_t screen_height)
{
    m_Viewport.x = x;
    m_Viewport.y = y;
    m_Viewport.width = width;
    m_Viewport.height = height;
    m_Viewport.near_z = near_z;
    m_Viewport.far_z = far_z;
    m_Viewport.view_angle = view_angle;
    m_Viewport.screen_width = screen_width;
    m_Viewport.screen_height = screen_height;

    M_Apply();
}

const VIEWPORT *Viewport_Get(void)
{
    return &m_Viewport;
}

void Viewport_AlterFOV(int16_t view_angle)
{
    m_Viewport.view_angle = view_angle;

    view_angle /= 2;

    g_PhdPersp =
        g_PhdWinWidth / 2 * Math_Cos(view_angle) / Math_Sin(view_angle);

    g_FltPersp = g_PhdPersp;
    g_FltRhwOPersp = g_RhwFactor / g_FltPersp;
    g_FltPerspONearZ = g_FltPersp / g_FltNearZ;

    double window_aspect_ratio = 4.0 / 3.0;
    if (!g_SavedAppSettings.fullscreen
        && g_SavedAppSettings.aspect_mode == AM_16_9) {
        window_aspect_ratio = 16.0 / 9.0;
    }

    g_ViewportAspectRatio =
        window_aspect_ratio / ((double)g_PhdWinWidth / (double)g_PhdWinHeight);
}

int16_t Viewport_GetFOV(void)
{
    return m_Viewport.view_angle == -1 ? Viewport_GetUserFOV()
                                       : m_Viewport.view_angle;
}

int16_t Viewport_GetUserFOV(void)
{
    return GAME_FOV * PHD_DEGREE;
}
