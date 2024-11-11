#include "game/viewport.h"

#include "config.h"
#include "game/math.h"
#include "game/output.h"
#include "global/const.h"
#include "global/vars.h"

#define MAP_GAME_VARS()                                                        \
    MAP_GAME_VAR(win_min_x, g_PhdWinMinX);                                     \
    MAP_GAME_VAR(win_min_y, g_PhdWinMinY);                                     \
    MAP_GAME_VAR(win_max_x, g_PhdWinMaxX);                                     \
    MAP_GAME_VAR(win_max_y, g_PhdWinMaxY);                                     \
    MAP_GAME_VAR(win_width, g_PhdWinWidth);                                    \
    MAP_GAME_VAR(win_height, g_PhdWinHeight);                                  \
    MAP_GAME_VAR(win_center_x, g_PhdWinCenterX);                               \
    MAP_GAME_VAR(win_center_y, g_PhdWinCenterY);                               \
    MAP_GAME_VAR(win_left, g_PhdWinLeft);                                      \
    MAP_GAME_VAR(win_top, g_PhdWinTop);                                        \
    MAP_GAME_VAR(win_right, g_PhdWinRight);                                    \
    MAP_GAME_VAR(win_bottom, g_PhdWinBottom);                                  \
    MAP_GAME_VAR(win_rect.left, g_PhdWinRect.left);                            \
    MAP_GAME_VAR(win_rect.bottom, g_PhdWinRect.bottom);                        \
    MAP_GAME_VAR(win_rect.top, g_PhdWinRect.top);                              \
    MAP_GAME_VAR(win_rect.right, g_PhdWinRect.right);                          \
    MAP_GAME_VAR(near_z, g_PhdNearZ);                                          \
    MAP_GAME_VAR(far_z, g_PhdFarZ);                                            \
    MAP_GAME_VAR(flt_near_z, g_FltNearZ);                                      \
    MAP_GAME_VAR(flt_far_z, g_FltFarZ);                                        \
    MAP_GAME_VAR(persp, g_PhdPersp);                                           \
    MAP_GAME_VAR(flt_res_z, g_FltResZ);                                        \
    MAP_GAME_VAR(flt_res_z_o_rhw, g_FltResZORhw);                              \
    MAP_GAME_VAR(flt_res_z_buf, g_FltResZBuf);                                 \
    MAP_GAME_VAR(flt_rhw_o_near_z, g_FltRhwONearZ);                            \
    MAP_GAME_VAR(flt_rhw_o_persp, g_FltRhwOPersp);                             \
    MAP_GAME_VAR(flt_persp, g_FltPersp);                                       \
    MAP_GAME_VAR(flt_persp_o_near_z, g_FltPerspONearZ);                        \
    MAP_GAME_VAR(view_distance, g_PhdViewDistance);                            \
    MAP_GAME_VAR(screen_width, g_PhdScreenWidth);                              \
    MAP_GAME_VAR(screen_height, g_PhdScreenHeight);                            \
    MAP_GAME_VAR(flt_win_left, g_FltWinLeft);                                  \
    MAP_GAME_VAR(flt_win_top, g_FltWinTop);                                    \
    MAP_GAME_VAR(flt_win_right, g_FltWinRight);                                \
    MAP_GAME_VAR(flt_win_bottom, g_FltWinBottom);                              \
    MAP_GAME_VAR(flt_win_center_x, g_FltWinCenterX);                           \
    MAP_GAME_VAR(flt_win_center_y, g_FltWinCenterY);                           \
    MAP_GAME_VAR(viewport_aspect_ratio, g_ViewportAspectRatio);

static VIEWPORT m_Viewport = { 0 };

static void M_AlterFov(VIEWPORT *vp);
static void M_InitGameVars(VIEWPORT *vp);
static void M_PullGameVars(VIEWPORT *vp);
static void M_ApplyGameVars(const VIEWPORT *vp);

static void M_AlterFov(VIEWPORT *const vp)
{
    vp->game_vars.persp = vp->game_vars.win_width / 2
        * Math_Cos(vp->view_angle / 2) / Math_Sin(vp->view_angle / 2);

    vp->game_vars.flt_persp = vp->game_vars.persp;
    vp->game_vars.flt_rhw_o_persp = g_RhwFactor / vp->game_vars.flt_persp;
    vp->game_vars.flt_persp_o_near_z =
        vp->game_vars.flt_persp / vp->game_vars.flt_near_z;

    double window_aspect_ratio = 4.0 / 3.0;
    if (!g_SavedAppSettings.fullscreen
        && g_SavedAppSettings.aspect_mode == AM_16_9) {
        window_aspect_ratio = 16.0 / 9.0;
    }

    vp->game_vars.viewport_aspect_ratio = window_aspect_ratio
        / ((double)vp->game_vars.win_width / (double)vp->game_vars.win_height);
}

static void M_InitGameVars(VIEWPORT *const vp)
{
    vp->game_vars.win_min_x = vp->x;
    vp->game_vars.win_min_y = vp->y;
    vp->game_vars.win_max_x = vp->width - 1;
    vp->game_vars.win_max_y = vp->height - 1;
    vp->game_vars.win_width = vp->width;
    vp->game_vars.win_height = vp->height;
    vp->game_vars.win_center_x = vp->width / 2;
    vp->game_vars.win_center_y = vp->height / 2;

    vp->game_vars.win_left = 0;
    vp->game_vars.win_top = 0;
    vp->game_vars.win_right = vp->game_vars.win_max_x;
    vp->game_vars.win_bottom = vp->game_vars.win_max_y;

    vp->game_vars.win_rect.left = vp->game_vars.win_min_x;
    vp->game_vars.win_rect.bottom =
        vp->game_vars.win_min_y + vp->game_vars.win_height;
    vp->game_vars.win_rect.top = vp->game_vars.win_min_y;
    vp->game_vars.win_rect.right =
        vp->game_vars.win_min_x + vp->game_vars.win_width;

    vp->game_vars.flt_win_left =
        vp->game_vars.win_min_x + vp->game_vars.win_left;
    vp->game_vars.flt_win_top = vp->game_vars.win_min_y + vp->game_vars.win_top;
    vp->game_vars.flt_win_right =
        vp->game_vars.win_min_x + vp->game_vars.win_right + 1;
    vp->game_vars.flt_win_bottom =
        vp->game_vars.win_min_y + vp->game_vars.win_bottom + 1;
    vp->game_vars.flt_win_center_x =
        vp->game_vars.win_min_x + vp->game_vars.win_center_x;
    vp->game_vars.flt_win_center_y =
        vp->game_vars.win_min_y + vp->game_vars.win_center_y;

    vp->game_vars.near_z = vp->near_z << W2V_SHIFT;
    vp->game_vars.far_z = vp->far_z << W2V_SHIFT;

    vp->game_vars.flt_near_z = vp->game_vars.near_z;
    vp->game_vars.flt_far_z = vp->game_vars.far_z;

    const double res_z = 0.99 * vp->game_vars.flt_near_z
        * vp->game_vars.flt_far_z
        / (vp->game_vars.flt_far_z - vp->game_vars.flt_near_z);
    vp->game_vars.flt_res_z = res_z;
    vp->game_vars.flt_res_z_o_rhw = res_z / g_RhwFactor;
    vp->game_vars.flt_res_z_buf = 0.005 + res_z / vp->game_vars.flt_near_z;
    vp->game_vars.flt_rhw_o_near_z = g_RhwFactor / vp->game_vars.flt_near_z;
    vp->game_vars.flt_persp = vp->game_vars.flt_persp;
    vp->game_vars.flt_persp_o_near_z =
        vp->game_vars.flt_persp / vp->game_vars.flt_near_z;
    vp->game_vars.view_distance = vp->far_z;

    vp->game_vars.screen_width = vp->screen_width;
    vp->game_vars.screen_height = vp->screen_height;

    M_AlterFov(vp);
}

static void M_PullGameVars(VIEWPORT *const vp)
{
#undef MAP_GAME_VAR
#define MAP_GAME_VAR(a, b) vp->game_vars.a = b;
    MAP_GAME_VARS();
}

static void M_ApplyGameVars(const VIEWPORT *const vp)
{
#undef MAP_GAME_VAR
#define MAP_GAME_VAR(a, b) b = vp->game_vars.a;
    MAP_GAME_VARS();
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

    M_InitGameVars(&m_Viewport);
    M_ApplyGameVars(&m_Viewport);
}

const VIEWPORT *Viewport_Get(void)
{
    M_PullGameVars(&m_Viewport);
    return &m_Viewport;
}

void Viewport_Restore(const VIEWPORT *ref_vp)
{
    memcpy(&m_Viewport, ref_vp, sizeof(VIEWPORT));
    M_ApplyGameVars(&m_Viewport);
}

void Viewport_AlterFOV(const int16_t view_angle)
{
    m_Viewport.view_angle = view_angle;

    M_PullGameVars(&m_Viewport);
    M_AlterFov(&m_Viewport);
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
