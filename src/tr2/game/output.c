#include "game/output.h"

#include "config.h"
#include "game/clock.h"
#include "game/inventory_ring.h"
#include "game/math.h"
#include "game/math_misc.h"
#include "game/matrix.h"
#include "game/random.h"
#include "game/render/common.h"
#include "game/render/priv.h"
#include "game/shell.h"
#include "game/viewport.h"
#include "global/vars.h"

#include <libtrx/log.h>

static int32_t m_TickComp = 0;

static int32_t M_CalcFogShade(int32_t depth);

static const int16_t *M_CalcRoomVerticesWibble(const int16_t *obj_ptr);

static void M_InsertBar(
    int32_t l, int32_t t, int32_t w, int32_t h, int32_t percent,
    INV_COLOR bar_color_main, INV_COLOR bar_color_highlight);

static int32_t M_CalcFogShade(const int32_t depth)
{
    if (depth > FOG_START) {
        return depth - FOG_START;
    }
    if (depth > FOG_END) {
        return 0x1FFF;
    }
    return 0;
}

static void M_InsertBar(
    const int32_t l, const int32_t t, const int32_t w, const int32_t h,
    const int32_t percent, const INV_COLOR bar_color_main,
    const INV_COLOR bar_color_highlight)
{
    const int32_t z_offset = 8;
    Render_InsertFlatRect(
        l, t, l + w, t + h, g_PhdNearZ + z_offset * 5,
        g_InvColors[INV_COLOR_WHITE]);
    Render_InsertFlatRect(
        l + 1, t + 1, l + w, t + h, g_PhdNearZ + z_offset * 4,
        g_InvColors[INV_COLOR_GRAY]);
    Render_InsertFlatRect(
        l + 1, t + 1, l + w - 1, t + h - 1, g_PhdNearZ + z_offset * 3,
        g_InvColors[INV_COLOR_BLACK]);

    Render_InsertFlatRect(
        l + 2, t + 2, l + (w - 2) * percent / 100, t + h - 2,
        g_PhdNearZ + z_offset * 2, g_InvColors[bar_color_main]);
    Render_InsertFlatRect(
        l + 2, t + 3, l + (w - 2) * percent / 100, t + 4,
        g_PhdNearZ + z_offset * 1, g_InvColors[bar_color_highlight]);
}

static const int16_t *M_CalcRoomVerticesWibble(const int16_t *obj_ptr)
{
    const int32_t vtx_count = *obj_ptr++;

    for (int32_t i = 0; i < vtx_count; i++) {
        PHD_VBUF *const vbuf = &g_PhdVBuf[i];
        if (obj_ptr[4] < 0) {
            obj_ptr += 6;
            continue;
        }

        double xs = vbuf->xs;
        double ys = vbuf->ys;
        xs += g_WibbleTable
            [(((g_WibbleOffset + (int)ys) % WIBBLE_SIZE) + WIBBLE_SIZE)
             % WIBBLE_SIZE];
        ys += g_WibbleTable
            [(((g_WibbleOffset + (int)xs) % WIBBLE_SIZE) + WIBBLE_SIZE)
             % WIBBLE_SIZE];

        int16_t clip_flags = vbuf->clip & ~15;
        if (xs < g_FltWinLeft) {
            clip_flags |= 1;
        } else if (xs > g_FltWinRight) {
            clip_flags |= 2;
        }

        if (ys < g_FltWinTop) {
            clip_flags |= 4;
        } else if (ys > g_FltWinBottom) {
            clip_flags |= 8;
        }
        vbuf->xs = xs;
        vbuf->ys = ys;
        vbuf->clip = clip_flags;
        obj_ptr += 6;
    }

    return obj_ptr;
}

void Output_InsertPolygons(const int16_t *obj_ptr, const int32_t clip)
{
    g_FltWinLeft = 0.0f;
    g_FltWinTop = 0.0f;
    g_FltWinRight = g_PhdWinMaxX + 1;
    g_FltWinBottom = g_PhdWinMaxY + 1;
    g_FltWinCenterX = g_PhdWinCenterX;
    g_FltWinCenterY = g_PhdWinCenterY;

    obj_ptr = Output_CalcObjectVertices(obj_ptr + 4);
    if (obj_ptr) {
        obj_ptr = Output_CalcVerticeLight(obj_ptr);
        obj_ptr = Render_InsertObjectGT4(obj_ptr + 1, *obj_ptr, ST_AVG_Z);
        obj_ptr = Render_InsertObjectGT3(obj_ptr + 1, *obj_ptr, ST_AVG_Z);
        obj_ptr = Render_InsertObjectG4(obj_ptr + 1, *obj_ptr, ST_AVG_Z);
        obj_ptr = Render_InsertObjectG3(obj_ptr + 1, *obj_ptr, ST_AVG_Z);
    }
}

void Output_InsertPolygons_I(const int16_t *const ptr, const int32_t clip)
{
    Matrix_Push();
    Matrix_Interpolate();
    Output_InsertPolygons(ptr, clip);
    Matrix_Pop();
}

void Output_InsertRoom(const int16_t *obj_ptr, int32_t is_outside)
{
    g_FltWinLeft = g_PhdWinLeft;
    g_FltWinTop = g_PhdWinTop;
    g_FltWinRight = g_PhdWinRight + 1;
    g_FltWinBottom = g_PhdWinBottom + 1;
    g_FltWinCenterX = g_PhdWinCenterX;
    g_FltWinCenterY = g_PhdWinCenterY;

    const int16_t *const old_obj_ptr = obj_ptr;
    obj_ptr = Output_CalcRoomVertices(obj_ptr, is_outside ? 0 : 16);

    if (g_IsWibbleEffect) {
        Render_EnableZBuffer(false, true);
        g_DiscardTransparent = true;
        obj_ptr = Render_InsertObjectGT4(obj_ptr + 1, *obj_ptr, ST_MAX_Z);
        obj_ptr = Render_InsertObjectGT3(obj_ptr + 1, *obj_ptr, ST_MAX_Z);
        g_DiscardTransparent = false;
        obj_ptr = M_CalcRoomVerticesWibble(old_obj_ptr);
        Render_EnableZBuffer(true, true);
    }

    obj_ptr = Render_InsertObjectGT4(obj_ptr + 1, *obj_ptr, ST_MAX_Z);
    obj_ptr = Render_InsertObjectGT3(obj_ptr + 1, *obj_ptr, ST_MAX_Z);

    Output_InsertRoomSprite(obj_ptr + 1, *obj_ptr);
}

void Output_InsertSkybox(const int16_t *obj_ptr)
{
    g_FltWinLeft = g_PhdWinLeft;
    g_FltWinTop = g_PhdWinTop;
    g_FltWinRight = g_PhdWinRight + 1;
    g_FltWinBottom = g_PhdWinBottom + 1;
    g_FltWinCenterX = g_PhdWinCenterX;
    g_FltWinCenterY = g_PhdWinCenterY;

    obj_ptr = Output_CalcObjectVertices(obj_ptr + 4);
    if (obj_ptr) {
        Render_EnableZBuffer(false, false);
        obj_ptr = Output_CalcSkyboxLight(obj_ptr);
        obj_ptr = Render_InsertObjectGT4(obj_ptr + 1, *obj_ptr, ST_FAR_Z);
        obj_ptr = Render_InsertObjectGT3(obj_ptr + 1, *obj_ptr, ST_FAR_Z);
        obj_ptr = Render_InsertObjectG4(obj_ptr + 1, *obj_ptr, ST_FAR_Z);
        obj_ptr = Render_InsertObjectG3(obj_ptr + 1, *obj_ptr, ST_FAR_Z);
        Render_EnableZBuffer(true, true);
    }
}

const int16_t *Output_CalcObjectVertices(const int16_t *obj_ptr)
{
    const double base_z = g_Config.rendering.enable_zbuffer
        ? 0.0
        : (g_MidSort << (W2V_SHIFT + 8));
    uint8_t total_clip = 0xFF;

    obj_ptr++; // skip poly counter
    const int32_t vtx_count = *obj_ptr++;

    for (int32_t i = 0; i < vtx_count; i++) {
        PHD_VBUF *const vbuf = &g_PhdVBuf[i];

        // clang-format off
        const MATRIX *const mptr = g_MatrixPtr;
        const double xv = (
            mptr->_00 * obj_ptr[0] +
            mptr->_01 * obj_ptr[1] +
            mptr->_02 * obj_ptr[2] +
            mptr->_03
        );
        const double yv = (
            mptr->_10 * obj_ptr[0] +
            mptr->_11 * obj_ptr[1] +
            mptr->_12 * obj_ptr[2] +
            mptr->_13
        );
        double zv = (
            mptr->_20 * obj_ptr[0] +
            mptr->_21 * obj_ptr[1] +
            mptr->_22 * obj_ptr[2] +
            mptr->_23
        );
        // clang-format on

        vbuf->xv = xv;
        vbuf->yv = yv;

        uint8_t clip_flags;
        if (zv < g_FltNearZ) {
            vbuf->zv = zv;
            clip_flags = 0x80;
        } else {
            if (zv >= g_FltFarZ) {
                vbuf->zv = g_FltFarZ;
            } else {
                vbuf->zv = zv + base_z;
            }

            const double persp = g_FltPersp / zv;
            vbuf->xs = persp * xv + g_FltWinCenterX;
            vbuf->ys = persp * yv + g_FltWinCenterY;
            vbuf->rhw = persp * g_FltRhwOPersp;

            clip_flags = 0x00;
            if (vbuf->xs < g_FltWinLeft) {
                clip_flags |= 1;
            } else if (vbuf->xs > g_FltWinRight) {
                clip_flags |= 2;
            }

            if (vbuf->ys < g_FltWinTop) {
                clip_flags |= 4;
            } else if (vbuf->ys > g_FltWinBottom) {
                clip_flags |= 8;
            }
        }

        vbuf->clip = clip_flags;
        total_clip &= clip_flags;
        obj_ptr += 3;
    }

    return total_clip == 0 ? obj_ptr : 0;
}

const int16_t *Output_CalcSkyboxLight(const int16_t *obj_ptr)
{
    int32_t count = *obj_ptr++;
    if (count > 0) {
        obj_ptr += 3 * count;
    } else if (count < 0) {
        count = -count;
        obj_ptr += count;
    }

    for (int32_t i = 0; i < count; i++) {
        g_PhdVBuf[i].g = 0xFFF;
    }

    return obj_ptr;
}

const int16_t *Output_CalcVerticeLight(const int16_t *obj_ptr)
{
    int32_t vtx_count = *obj_ptr++;

    if (vtx_count > 0) {
        if (g_LsDivider) {
            // clang-format off
            const MATRIX *const mptr = g_MatrixPtr;
            int32_t xv = (
                g_LsVectorView.x * mptr->_00 +
                g_LsVectorView.y * mptr->_10 +
                g_LsVectorView.z * mptr->_20
            ) / g_LsDivider;
            int32_t yv = (
                g_LsVectorView.x * mptr->_01 +
                g_LsVectorView.y * mptr->_11 +
                g_LsVectorView.z * mptr->_21
            ) / g_LsDivider;
            int32_t zv = (
                g_LsVectorView.x * mptr->_02 +
                g_LsVectorView.y * mptr->_12 +
                g_LsVectorView.z * mptr->_22
            ) / g_LsDivider;
            // clang-format on

            for (int32_t i = 0; i < vtx_count; i++) {
                int16_t shade = g_LsAdder
                    + ((xv * obj_ptr[0] + yv * obj_ptr[1] + zv * obj_ptr[2])
                       >> 16);
                CLAMP(shade, 0, 0x1FFF);

                g_PhdVBuf[i].g = shade;
                obj_ptr += 3;
            }
        } else {
            int16_t shade = g_LsAdder;
            CLAMP(shade, 0, 0x1FFF);
            obj_ptr += 3 * vtx_count;
            for (int32_t i = 0; i < vtx_count; i++) {
                g_PhdVBuf[i].g = shade;
            }
        }
    } else if (vtx_count < 0) {
        for (int32_t i = 0; i < -vtx_count; i++) {
            int16_t shade = g_LsAdder + *obj_ptr++;
            CLAMP(shade, 0, 0x1FFF);
            g_PhdVBuf[i].g = shade;
        }
    }

    return obj_ptr;
}

const int16_t *Output_CalcRoomVertices(const int16_t *obj_ptr, int32_t far_clip)
{
    const double base_z = g_Config.rendering.enable_zbuffer
        ? 0.0
        : (g_MidSort << (W2V_SHIFT + 8));
    const int32_t vtx_count = *obj_ptr++;

    for (int32_t i = 0; i < vtx_count; i++) {
        PHD_VBUF *const vbuf = &g_PhdVBuf[i];

        // clang-format off
        const MATRIX *const mptr = g_MatrixPtr;
        const double xv = (
            mptr->_00 * obj_ptr[0] +
            mptr->_01 * obj_ptr[1] +
            mptr->_02 * obj_ptr[2] +
            mptr->_03
        );
        const double yv = (
            mptr->_10 * obj_ptr[0] +
            mptr->_11 * obj_ptr[1] +
            mptr->_12 * obj_ptr[2] +
            mptr->_13
        );
        const int32_t zv_int = (
            mptr->_20 * obj_ptr[0] +
            mptr->_21 * obj_ptr[1] +
            mptr->_22 * obj_ptr[2] +
            mptr->_23
        );
        const double zv = zv_int;
        // clang-format on

        vbuf->xv = xv;
        vbuf->yv = yv;
        vbuf->zv = zv;

        int16_t shade = obj_ptr[5];
        if (g_IsWaterEffect) {
            shade += g_ShadesTable
                [((uint8_t)g_WibbleOffset
                  + (uint8_t)g_RandomTable[(vtx_count - i) % WIBBLE_SIZE])
                 % WIBBLE_SIZE];
        }

        uint16_t clip_flags = 0;
        if (zv < g_FltNearZ) {
            clip_flags = 0xFF80;
        } else {
            const double persp = g_FltPersp / zv;
            const int32_t depth = zv_int >> W2V_SHIFT;
            vbuf->zv += base_z;

            if (depth < FOG_END) {
                if (depth > FOG_START) {
                    shade += depth - FOG_START;
                }
                vbuf->rhw = persp * g_FltRhwOPersp;
            } else {
                // clip_flags = far_clip;
                shade = 0x1FFF;
                vbuf->zv = g_FltFarZ;
                vbuf->rhw = persp * g_FltRhwOPersp;
            }

            double xs = xv * persp + g_FltWinCenterX;
            double ys = yv * persp + g_FltWinCenterY;

            if (xs < g_FltWinLeft) {
                clip_flags |= 1;
            } else if (xs > g_FltWinRight) {
                clip_flags |= 2;
            }

            if (ys < g_FltWinTop) {
                clip_flags |= 4;
            } else if (ys > g_FltWinBottom) {
                clip_flags |= 8;
            }

            vbuf->xs = xs;
            vbuf->ys = ys;
            // clip_flags |= (~((uint8_t)(vbuf->zv / 0x155555.p0))) << 8;
        }

        CLAMP(shade, 0, 0x1FFF);
        vbuf->g = shade;
        vbuf->clip = clip_flags;
        obj_ptr += 6;
    }

    return obj_ptr;
}

void Output_RotateLight(int16_t pitch, int16_t yaw)
{
    g_LsYaw = yaw;
    g_LsPitch = pitch;

    int32_t xcos = Math_Cos(pitch);
    int32_t xsin = Math_Sin(pitch);
    int32_t wcos = Math_Cos(yaw);
    int32_t wsin = Math_Sin(yaw);

    int32_t x = (xcos * wsin) >> W2V_SHIFT;
    int32_t y = -xsin;
    int32_t z = (xcos * wcos) >> W2V_SHIFT;

    const MATRIX *const m = &g_W2VMatrix;
    g_LsVectorView.x = (m->_00 * x + m->_01 * y + m->_02 * z) >> W2V_SHIFT;
    g_LsVectorView.y = (m->_10 * x + m->_11 * y + m->_12 * z) >> W2V_SHIFT;
    g_LsVectorView.z = (m->_20 * x + m->_21 * y + m->_22 * z) >> W2V_SHIFT;
}

void Output_AlterFOV(int16_t fov)
{
    Viewport_AlterFOV(fov);
}

const int16_t *Output_InsertRoomSprite(
    const int16_t *obj_ptr, const int32_t vtx_count)
{
    for (int32_t i = 0; i < vtx_count; i++) {
        const PHD_VBUF *vbuf = &g_PhdVBuf[*obj_ptr++];
        const int16_t sprite_idx = *obj_ptr++;
        if ((int8_t)vbuf->clip < 0) {
            continue;
        }

        const PHD_SPRITE *const sprite = &g_PhdSprites[sprite_idx];
        const double persp = (double)(vbuf->zv / g_PhdPersp);
        const double x0 =
            g_PhdWinCenterX + (vbuf->xv + (sprite->x0 << W2V_SHIFT)) / persp;
        const double y0 =
            g_PhdWinCenterY + (vbuf->yv + (sprite->y0 << W2V_SHIFT)) / persp;
        const double x1 =
            g_PhdWinCenterX + (vbuf->xv + (sprite->x1 << W2V_SHIFT)) / persp;
        const double y1 =
            g_PhdWinCenterY + (vbuf->yv + (sprite->y1 << W2V_SHIFT)) / persp;
        if (x1 >= g_PhdWinLeft && y1 >= g_PhdWinTop && x0 < g_PhdWinRight
            && y0 < g_PhdWinBottom) {
            Render_InsertSprite(vbuf->zv, x0, y0, x1, y1, sprite_idx, vbuf->g);
        }
    }

    return obj_ptr;
}

void Output_DrawSprite(
    const uint32_t flags, int32_t x, int32_t y, int32_t z,
    const int16_t sprite_idx, int16_t shade, const int16_t scale)
{
    const MATRIX *const mptr = g_MatrixPtr;

    int32_t xv;
    int32_t yv;
    int32_t zv;
    if (flags & SPRF_ABS) {
        x -= g_W2VMatrix._03;
        y -= g_W2VMatrix._13;
        z -= g_W2VMatrix._23;
        if (x < -g_PhdViewDistance || x > g_PhdViewDistance
            || y < -g_PhdViewDistance || y > g_PhdViewDistance
            || z < -g_PhdViewDistance || z > g_PhdViewDistance) {
            return;
        }

        zv = g_W2VMatrix._22 * z + g_W2VMatrix._21 * y + g_W2VMatrix._20 * x;
        if (zv < g_PhdNearZ || zv >= g_PhdFarZ) {
            return;
        }
        xv = g_W2VMatrix._02 * z + g_W2VMatrix._01 * y + g_W2VMatrix._00 * x;
        yv = g_W2VMatrix._12 * z + g_W2VMatrix._11 * y + g_W2VMatrix._10 * x;
    } else if (x | y | z) {
        zv = x * mptr->_20 + y * mptr->_21 + z * mptr->_22 + mptr->_23;
        if (zv < g_PhdNearZ || zv > g_PhdFarZ) {
            return;
        }
        xv = x * mptr->_00 + y * mptr->_01 + z * mptr->_02 + mptr->_03;
        yv = x * mptr->_10 + y * mptr->_11 + z * mptr->_12 + mptr->_13;
    } else {
        zv = mptr->_23;
        if (zv < g_PhdNearZ || zv > g_PhdFarZ) {
            return;
        }
        xv = mptr->_03;
        yv = mptr->_13;
    }

    const PHD_SPRITE *const sprite = &g_PhdSprites[sprite_idx];
    int32_t x0 = sprite->x0;
    int32_t y0 = sprite->y0;
    int32_t x1 = sprite->x1;
    int32_t y1 = sprite->y1;

    if (flags & SPRF_SCALE) {
        x0 = (x0 * scale) << (W2V_SHIFT - 8);
        y0 = (y0 * scale) << (W2V_SHIFT - 8);
        x1 = (x1 * scale) << (W2V_SHIFT - 8);
        y1 = (y1 * scale) << (W2V_SHIFT - 8);
    } else {
        x0 <<= W2V_SHIFT;
        y0 <<= W2V_SHIFT;
        x1 <<= W2V_SHIFT;
        y1 <<= W2V_SHIFT;
    }

    int32_t zp = zv / g_PhdPersp;

    x0 = g_PhdWinCenterX + (x0 + xv) / zp;
    if (x0 >= g_PhdWinWidth) {
        return;
    }

    y0 = g_PhdWinCenterY + (y0 + yv) / zp;
    if (y0 >= g_PhdWinHeight) {
        return;
    }

    x1 = g_PhdWinCenterX + (x1 + xv) / zp;
    if (x1 < 0) {
        return;
    }

    y1 = g_PhdWinCenterY + (y1 + yv) / zp;
    if (y1 < 0) {
        return;
    }

    if (flags & SPRF_SHADE) {
        const int32_t depth = zv >> W2V_SHIFT;
        if (depth > FOG_START) {
            shade += depth - FOG_START;
            if (shade > 0x1FFF) {
                return;
            }
        }
    } else {
        shade = 0x1000;
    }

    Render_InsertSprite(zv, x0, y0, x1, y1, sprite_idx, shade);
}

void Output_DrawPickup(
    const int32_t sx, const int32_t sy, const int32_t scale,
    const int16_t sprite_idx, const int16_t shade)
{
    const PHD_SPRITE *const sprite = &g_PhdSprites[sprite_idx];
    const int32_t x0 = sx + ((sprite->x0 * scale) / PHD_ONE);
    const int32_t y0 = sy + ((sprite->y0 * scale) / PHD_ONE);
    const int32_t x1 = sx + ((sprite->x1 * scale) / PHD_ONE);
    const int32_t y1 = sy + ((sprite->y1 * scale) / PHD_ONE);
    if (x1 >= 0 && y1 >= 0 && x0 < g_PhdWinWidth && y0 < g_PhdWinHeight) {
        Render_InsertSprite(200, x0, y0, x1, y1, sprite_idx, shade);
    }
}

void Output_DrawScreenSprite2D(
    const int32_t sx, const int32_t sy, const int32_t sz, const int32_t scale_h,
    const int32_t scale_v, const int16_t sprite_idx, const int16_t shade,
    const uint16_t flags)
{
    const PHD_SPRITE *const sprite = &g_PhdSprites[sprite_idx];
    const int32_t x0 = sx + ((sprite->x0 * scale_h) / PHD_ONE);
    const int32_t y0 = sy + ((sprite->y0 * scale_v) / PHD_ONE);
    const int32_t x1 = sx + ((sprite->x1 * scale_h) / PHD_ONE);
    const int32_t y1 = sy + ((sprite->y1 * scale_v) / PHD_ONE);
    const int32_t z = g_PhdNearZ + sz * 8;
    if (x1 >= 0 && y1 >= 0 && x0 < g_PhdWinWidth && y0 < g_PhdWinHeight) {
        Render_InsertSprite(z, x0, y0, x1, y1, sprite_idx, shade);
    }
}

void Output_DrawScreenSprite(
    const int32_t sx, const int32_t sy, const int32_t sz, const int32_t scale_h,
    const int32_t scale_v, const int16_t sprite_idx, const int16_t shade,
    const uint16_t flags)
{
    const PHD_SPRITE *const sprite = &g_PhdSprites[sprite_idx];
    const int32_t x0 = sx + (((sprite->x0 / 8) * scale_h) / PHD_ONE);
    const int32_t x1 = sx + (((sprite->x1 / 8) * scale_h) / PHD_ONE);
    const int32_t y0 = sy + (((sprite->y0 / 8) * scale_v) / PHD_ONE);
    const int32_t y1 = sy + (((sprite->y1 / 8) * scale_v) / PHD_ONE);
    const int32_t z = sz * 8;
    if (x1 >= 0 && y1 >= 0 && x0 < g_PhdWinWidth && y0 < g_PhdWinHeight) {
        Render_InsertSprite(z, x0, y0, x1, y1, sprite_idx, shade);
    }
}

bool Output_MakeScreenshot(const char *const path)
{
    LOG_INFO("Taking screenshot");
    GFX_Context_ScheduleScreenshot(path);
    return true;
}

void Output_BeginScene(void)
{
    Matrix_ResetStack();
    Text_DrawReset();
    Render_BeginScene();
}

int32_t Output_EndScene(const bool sync_ticks)
{
    const int32_t passed = sync_ticks ? Clock_WaitTick() : 0;
    Render_EndScene();
    Shell_ProcessEvents();
    return passed;
}

void Output_LoadBackgroundFromFile(const char *const file_name)
{
    IMAGE *const image = Image_CreateFromFile(file_name);
    Render_LoadBackgroundFromImage(image);
    Image_Free(image);
}

void Output_LoadBackgroundFromObject(void)
{
    const OBJECT *const obj = &g_Objects[O_INV_BACKGROUND];
    if (!obj->loaded) {
        return;
    }

    const int16_t *mesh_ptr = g_Meshes[obj->mesh_idx];
    mesh_ptr += 5;
    const int32_t num_vertices = *mesh_ptr++;
    mesh_ptr += num_vertices * 3;

    const int32_t num_normals = *mesh_ptr++;
    if (num_normals >= 0) {
        mesh_ptr += num_normals * 3;
    } else {
        mesh_ptr -= num_normals;
    }
    const int32_t num_quads = *mesh_ptr++;
    if (num_quads < 1) {
        return;
    }

    mesh_ptr += 4;

    const int32_t texture_idx = *mesh_ptr++;
    const PHD_TEXTURE *const texture = &g_TextureInfo[texture_idx];
    Render_LoadBackgroundFromTexture(texture, 8, 6);
    return;
}

void Output_UnloadBackground(void)
{
    Render_UnloadBackground();
}

void Output_InsertBackPolygon(
    const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1)
{
    Render_InsertFlatRect(x0, y0, x1, y1, g_PhdFarZ + 1, g_InvColors[0]);
}

void Output_DrawBlackRectangle(int32_t opacity)
{
    Render_DrawBlackRectangle(opacity);
}

void Output_DrawBackground(void)
{
    Render_DrawBackground();
}

void Output_DrawPolyList(void)
{
    Render_DrawPolyList();
}

void Output_DrawScreenLine(
    const int32_t x, const int32_t y, const int32_t z, const int32_t x_len,
    const int32_t y_len, const uint8_t color_idx, const void *const gour,
    const uint16_t flags)
{
    Render_InsertLine(
        x, y, x + x_len, y + y_len, g_PhdNearZ + 8 * z, color_idx);
}

void Output_DrawScreenBox(
    const int32_t sx, const int32_t sy, const int32_t z, const int32_t width,
    const int32_t height, const uint8_t color_idx, const void *const gour,
    const uint16_t flags)
{
    const int32_t col_1 = 15;
    const int32_t col_2 = 31;
    Output_DrawScreenLine(sx, sy - 1, z, width + 1, 0, col_1, NULL, flags);
    Output_DrawScreenLine(sx + 1, sy, z, width - 1, 0, col_2, NULL, flags);
    Output_DrawScreenLine(
        sx + width, sy + 1, z, 0, height - 1, col_1, NULL, flags);
    Output_DrawScreenLine(
        sx + width + 1, sy, z, 0, height + 1, col_2, NULL, flags);
    Output_DrawScreenLine(sx - 1, sy - 1, z, 0, height + 1, col_1, NULL, flags);
    Output_DrawScreenLine(sx, sy, z, 0, height - 1, col_2, NULL, flags);
    Output_DrawScreenLine(sx, height + sy, z, width - 1, 0, col_1, NULL, flags);
    Output_DrawScreenLine(
        sx - 1, height + sy + 1, z, width + 1, 0, col_2, NULL, flags);
}

void Output_DrawScreenFBox(
    const int32_t sx, const int32_t sy, const int32_t z, const int32_t width,
    const int32_t height, const uint8_t color_idx, const void *const gour,
    const uint16_t flags)
{
    Render_InsertTransQuad(sx, sy, width + 1, height + 1, g_PhdNearZ + 8 * z);
}

void Output_DrawHealthBar(const int32_t percent)
{
    g_IsShadeEffect = false;
    M_InsertBar(6, 6, 105, 9, percent, INV_COLOR_RED, INV_COLOR_ORANGE);
}

void Output_DrawAirBar(const int32_t percent)
{
    g_IsShadeEffect = false;
    M_InsertBar(
        g_PhdWinWidth - 112, 6, 105, 9, percent, INV_COLOR_BLUE,
        INV_COLOR_WHITE);
}

int16_t Output_FindColor(
    const int32_t red, const int32_t green, const int32_t blue)
{
    int32_t best_idx = 0;
    int32_t best_diff = INT32_MAX;
    for (int32_t i = 0; i < 256; i++) {
        const int32_t dr = red - g_GamePalette8[i].red;
        const int32_t dg = green - g_GamePalette8[i].green;
        const int32_t db = blue - g_GamePalette8[i].blue;
        const int32_t diff = SQUARE(dr) + SQUARE(dg) + SQUARE(db);
        if (diff < best_diff) {
            best_diff = diff;
            best_idx = i;
        }
    }

    return best_idx;
}

void Output_DoAnimateTextures(const int32_t ticks)
{
    m_TickComp += ticks;
    while (m_TickComp > TICKS_PER_FRAME * 5) {
        const int16_t *ptr = g_AnimTextureRanges;
        int16_t i = *(ptr++);
        for (; i > 0; --i, ++ptr) {
            int16_t j = *(ptr++);
            const PHD_TEXTURE temp = g_TextureInfo[*ptr];
            for (; j > 0; --j, ++ptr) {
                g_TextureInfo[ptr[0]] = g_TextureInfo[ptr[1]];
            }
            g_TextureInfo[*ptr] = temp;
        }
        m_TickComp -= TICKS_PER_FRAME * 5;
    }
}

void Output_InsertShadow(
    int16_t radius, const BOUNDS_16 *bounds, const ITEM *item)
{
    const int32_t x1 = bounds->min_x;
    const int32_t x2 = bounds->max_x;
    const int32_t z1 = bounds->min_z;
    const int32_t z2 = bounds->max_z;
    const int32_t mid_x = (x1 + x2) / 2;
    const int32_t mid_z = (z1 + z2) / 2;
    const int32_t x_add = radius * (x2 - x1) / 1024;
    const int32_t z_add = radius * (z2 - z1) / 1024;

    struct {
        int16_t poly_count;
        int16_t vertex_count;
        XYZ_16 vertex[8];
    } shadow_info = {
        .poly_count = 1,
        .vertex_count = 8,
        .vertex = {
            { .x = mid_x - x_add, .z = mid_z + z_add * 2 },
            { .x = mid_x + x_add, .z = mid_z + z_add * 2 },
            { .x = mid_x + x_add * 2, .z = z_add + mid_z },
            { .x = mid_x + x_add * 2, .z = mid_z - z_add },
            { .x = mid_x + x_add, .z = mid_z - 2 * z_add },
            { .x = mid_x - x_add, .z = mid_z - 2 * z_add },
            { .x = mid_x - 2 * x_add, .z = mid_z - z_add },
            { .x = mid_x - 2 * x_add, .z = z_add + mid_z },
        },
    };

    g_FltWinLeft = (float)(g_PhdWinLeft);
    g_FltWinTop = (float)(g_PhdWinTop);
    g_FltWinRight = (float)(g_PhdWinRight + 1);
    g_FltWinBottom = (float)(g_PhdWinBottom + 1);
    g_FltWinCenterX = (float)(g_PhdWinCenterX);
    g_FltWinCenterY = (float)(g_PhdWinCenterY);

    Matrix_Push();
    Matrix_TranslateAbs(item->pos.x, item->floor, item->pos.z);
    Matrix_RotY(item->rot.y);
    if (Output_CalcObjectVertices((int16_t *)&shadow_info)) {
        Render_InsertTransOctagon(g_PhdVBuf, 24);
    }
    Matrix_Pop();
}

void Output_CalculateWibbleTable(void)
{
    for (int32_t i = 0; i < WIBBLE_SIZE; i++) {
        const int32_t sine = Math_Sin(i * PHD_360 / WIBBLE_SIZE);
        g_WibbleTable[i] = (sine * MAX_WIBBLE) >> W2V_SHIFT;
        g_ShadesTable[i] = (sine * MAX_SHADE) >> W2V_SHIFT;
        g_RandomTable[i] = (Random_GetDraw() >> 5) - 0x01FF;
        for (int32_t j = 0; j < WIBBLE_SIZE; j++) {
            g_RoomLightTables[i].table[j] = (j - (WIBBLE_SIZE / 2)) * i
                * MAX_ROOM_LIGHT_UNIT / (WIBBLE_SIZE - 1);
        }
    }
}

int32_t Output_GetObjectBounds(const BOUNDS_16 *const bounds)
{
    const MATRIX *const m = g_MatrixPtr;
    if (m->_23 >= g_PhdFarZ) {
        return 0;
    }

    constexpr int32_t vtx_count = 8;
    const XYZ_32 vtx[vtx_count] = {
        { .x = bounds->min_x, .y = bounds->min_y, .z = bounds->min_z },
        { .x = bounds->max_x, .y = bounds->min_y, .z = bounds->min_z },
        { .x = bounds->max_x, .y = bounds->max_y, .z = bounds->min_z },
        { .x = bounds->min_x, .y = bounds->max_y, .z = bounds->min_z },
        { .x = bounds->min_x, .y = bounds->min_y, .z = bounds->max_z },
        { .x = bounds->max_x, .y = bounds->min_y, .z = bounds->max_z },
        { .x = bounds->max_x, .y = bounds->max_y, .z = bounds->max_z },
        { .x = bounds->min_x, .y = bounds->max_y, .z = bounds->max_z },
    };

    int32_t y_min = 0x3FFFFFFF;
    int32_t x_min = 0x3FFFFFFF;
    int32_t y_max = -0x3FFFFFFF;
    int32_t x_max = -0x3FFFFFFF;

    int32_t num_z = 0;
    for (int32_t i = 0; i < vtx_count; i++) {
        const int32_t x = vtx[i].x;
        const int32_t y = vtx[i].y;
        const int32_t z = vtx[i].z;

        const int32_t zv = x * m->_20 + y * m->_21 + z * m->_22 + m->_23;
        if (zv <= g_PhdNearZ || zv >= g_PhdFarZ) {
            continue;
        }

        num_z++;
        const int32_t zp = zv / g_PhdPersp;
        const int32_t xv = (x * m->_00 + y * m->_01 + z * m->_02 + m->_03) / zp;
        const int32_t yv = (x * m->_10 + y * m->_11 + z * m->_12 + m->_13) / zp;
        CLAMPG(x_min, xv);
        CLAMPL(x_max, xv);
        CLAMPG(y_min, yv);
        CLAMPL(y_max, yv);
    }

    x_min += g_PhdWinCenterX;
    x_max += g_PhdWinCenterX;
    y_min += g_PhdWinCenterY;
    y_max += g_PhdWinCenterY;

    if (num_z == 0 || x_min > g_PhdWinRight || y_min > g_PhdWinBottom
        || x_max < g_PhdWinLeft || y_max < g_PhdWinTop) {
        // out of screen
        return 0;
    }

    if (num_z < 8 || x_min < 0 || y_min < 0 || x_max > g_PhdWinMaxX
        || y_max > g_PhdWinMaxY) {
        // clipped
        return -1;
    }

    // fully on screen
    return 1;
}

void Output_CalculateLight(
    const int32_t x, const int32_t y, const int32_t z, const int16_t room_num)
{
    const ROOM *const r = &g_Rooms[room_num];

    int32_t brightest_shade = 0;
    XYZ_32 brightest_pos = { 0 };

    if (r->light_mode != 0) {
        const int32_t light_shade = g_RoomLightShades[r->light_mode];
        for (int32_t i = 0; i < r->num_lights; i++) {
            const LIGHT *const light = &r->lights[i];
            const int32_t dx = x - light->pos.x;
            const int32_t dy = y - light->pos.y;
            const int32_t dz = z - light->pos.z;

            const int32_t falloff_1 = SQUARE(light->falloff_1) >> 12;
            const int32_t falloff_2 = SQUARE(light->falloff_2) >> 12;
            const int32_t dist = (SQUARE(dx) + SQUARE(dy) + SQUARE(dz)) >> 12;

            const int32_t shade_1 =
                falloff_1 * light->intensity_1 / (falloff_1 + dist);
            const int32_t shade_2 =
                falloff_2 * light->intensity_2 / (falloff_2 + dist);
            const int32_t shade =
                shade_1 + (shade_2 - shade_1) * light_shade / (WIBBLE_SIZE - 1);

            if (shade > brightest_shade) {
                brightest_shade = shade;
                brightest_pos = light->pos;
            }
        }
    } else {
        for (int32_t i = 0; i < r->num_lights; i++) {
            const LIGHT *const light = &r->lights[i];
            const int32_t dx = x - light->pos.x;
            const int32_t dy = y - light->pos.y;
            const int32_t dz = z - light->pos.z;
            const int32_t falloff_1 =
                (light->falloff_1 * light->falloff_1) >> 12;
            const int32_t dist = (SQUARE(dx) + SQUARE(dy) + SQUARE(dz)) >> 12;
            const int32_t shade =
                falloff_1 * light->intensity_1 / (falloff_1 + dist);
            if (shade > brightest_shade) {
                brightest_shade = shade;
                brightest_pos = light->pos;
            }
        }
    }

    int32_t adder = brightest_shade;
    for (int32_t i = 0; i < g_DynamicLightCount; i++) {
        const LIGHT *const light = &g_DynamicLights[i];
        const int32_t dx = x - light->pos.x;
        const int32_t dy = y - light->pos.y;
        const int32_t dz = z - light->pos.z;
        const int32_t radius = 1 << light->falloff_1;
        if (dx < -radius || dx > radius || dy < -radius || dy > radius
            || dz < -radius || dz > radius) {
            continue;
        }

        const int32_t dist = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
        if (dist > SQUARE(radius)) {
            continue;
        }

        const int32_t shade = (1 << light->intensity_1)
            - (dist >> (2 * light->falloff_1 - light->intensity_1));
        if (shade > brightest_shade) {
            brightest_shade = shade;
            brightest_pos = light->pos;
        }
        adder += shade;
    }

    adder /= 2;
    if (adder != 0) {
        g_LsAdder = r->ambient_1 - adder;
        g_LsDivider = (1 << (W2V_SHIFT + 12)) / adder;
        int16_t angles[2];
        Math_GetVectorAngles(
            x - brightest_pos.x, y - brightest_pos.y, z - brightest_pos.z,
            angles);
        Output_RotateLight(angles[1], angles[0]);
    } else {
        g_LsAdder = r->ambient_1;
        g_LsDivider = 0;
    }

    const int32_t depth = g_MatrixPtr->_23 >> W2V_SHIFT;
    g_LsAdder += M_CalcFogShade(depth);
    CLAMPG(g_LsAdder, 0x1FFF);
}

void Output_CalculateStaticLight(const int16_t adder)
{
    g_LsAdder = adder - 0x1000;
    const int32_t depth = g_MatrixPtr->_23 >> W2V_SHIFT;
    g_LsAdder += M_CalcFogShade(depth);
    CLAMPG(g_LsAdder, 0x1FFF);
}

void Output_CalculateStaticMeshLight(
    const int32_t x, const int32_t y, const int32_t z, const int32_t shade_1,
    const int32_t shade_2, const ROOM *const room)
{
    int32_t adder = shade_1;
    if (room->light_mode != 0) {
        adder += (shade_2 - shade_1) * g_RoomLightShades[room->light_mode]
            / (WIBBLE_SIZE - 1);
    }

    for (int32_t i = 0; i < g_DynamicLightCount; i++) {
        const LIGHT *const light = &g_DynamicLights[i];
        const int32_t dx = x - light->pos.x;
        const int32_t dy = y - light->pos.y;
        const int32_t dz = z - light->pos.z;
        const int32_t radius = 1 << light->falloff_1;
        if (dx < -radius || dx > radius || dy < -radius || dy > radius
            || dz < -radius || dz > radius) {
            continue;
        }

        const int32_t dist = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
        if (dist > SQUARE(radius)) {
            continue;
        }

        const int32_t shade = (1 << light->intensity_1)
            - (dist >> (2 * light->falloff_1 - light->intensity_1));
        adder -= shade;
        if (adder < 0) {
            break;
        }
    }

    Output_CalculateStaticLight(adder);
}

void Output_CalculateObjectLighting(
    const ITEM *const item, const BOUNDS_16 *const bounds)
{
    if (item->shade_1 >= 0) {
        Output_CalculateStaticMeshLight(
            item->pos.x, item->pos.y, item->pos.z, item->shade_1, item->shade_2,
            &g_Rooms[item->room_num]);
        return;
    }

    Matrix_PushUnit();

    Matrix_TranslateSet(0, 0, 0);
    Matrix_RotYXZ(item->rot.y, item->rot.x, item->rot.z);
    Matrix_TranslateRel(
        (bounds->min_x + bounds->max_x) / 2,
        (bounds->max_y + bounds->min_y) / 2,
        (bounds->max_z + bounds->min_z) / 2);
    const XYZ_32 pos = {
        .x = item->pos.x + (g_MatrixPtr->_03 >> W2V_SHIFT),
        .y = item->pos.y + (g_MatrixPtr->_13 >> W2V_SHIFT),
        .z = item->pos.z + (g_MatrixPtr->_23 >> W2V_SHIFT),
    };
    Matrix_Pop();

    Output_CalculateLight(pos.x, pos.y, pos.z, item->room_num);
}

void Output_LightRoom(ROOM *const room)
{
    if (room->light_mode != 0) {
        const ROOM_LIGHT_TABLE *const light_table =
            &g_RoomLightTables[g_RoomLightShades[room->light_mode]];
        int32_t vtx_count = *room->data;
        ROOM_VERTEX *const vtx = (ROOM_VERTEX *)(room->data + 1);
        for (int32_t i = 0; i < vtx_count; i++) {
            const int32_t wibble =
                light_table->table[vtx[i].light_table_value % WIBBLE_SIZE];
            vtx[i].light_adder = vtx[i].light_base + wibble;
        }
    } else if (room->flags & RF_DYNAMIC_LIT) {
        const int32_t vtx_count = *room->data;
        ROOM_VERTEX *const vtx = (ROOM_VERTEX *)(room->data + 1);
        for (int32_t i = 0; i < vtx_count; i++) {
            vtx[i].light_adder = vtx[i].light_base;
        }
        room->flags &= ~RF_DYNAMIC_LIT;
    }

    const int32_t x_min = WALL_L;
    const int32_t z_min = WALL_L;
    const int32_t x_max = (room->size.x - 1) * WALL_L;
    const int32_t z_max = (room->size.z - 1) * WALL_L;

    for (int32_t i = 0; i < g_DynamicLightCount; i++) {
        const LIGHT *const light = &g_DynamicLights[i];
        const int32_t x = light->pos.x - room->pos.x;
        const int32_t y = light->pos.y;
        const int32_t z = light->pos.z - room->pos.z;
        const int32_t radius = 1 << light->falloff_1;
        if (x - radius > x_max || z - radius > z_max || x + radius < x_min
            || z + radius < z_min) {
            continue;
        }

        room->flags |= RF_DYNAMIC_LIT;

        const int32_t vtx_count = *room->data;
        ROOM_VERTEX *const vtx = (ROOM_VERTEX *)(room->data + 1);
        for (int32_t j = 0; j < vtx_count; j++) {
            ROOM_VERTEX *const v = &vtx[j];
            if (v->light_adder == 0) {
                continue;
            }

            const int32_t dx = v->pos.x - x;
            const int32_t dy = v->pos.y - y;
            const int32_t dz = v->pos.z - z;
            if (dx < -radius || dx > radius || dy < -radius || dy > radius
                || dz < -radius || dz > radius) {
                continue;
            }

            const int32_t dist = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
            if (dist > SQUARE(radius)) {
                continue;
            }

            const int32_t shade = (1 << light->intensity_1)
                - (dist >> (2 * light->falloff_1 - light->intensity_1));
            v->light_adder -= shade;
            CLAMPL(v->light_adder, 0);
        }
    }
}

void Output_SetupBelowWater(const bool is_underwater)
{
    g_IsWet = is_underwater;
    Render_SetWet(is_underwater);
    g_IsWaterEffect = true;
    g_IsWibbleEffect = !is_underwater;
    g_IsShadeEffect = true;
}

void Output_SetupAboveWater(const bool is_underwater)
{
    g_IsWibbleEffect = is_underwater;
    g_IsWaterEffect = false;
    g_IsShadeEffect = is_underwater;
}

void Output_AnimateTextures(const int32_t ticks)
{
    g_WibbleOffset = (g_WibbleOffset + (ticks / TICKS_PER_FRAME)) % WIBBLE_SIZE;
    g_RoomLightShades[1] = Random_GetDraw() % WIBBLE_SIZE;
    g_RoomLightShades[2] = (WIBBLE_SIZE - 1)
            * (Math_Sin((g_WibbleOffset * PHD_360) / WIBBLE_SIZE) + 0x4000)
        >> 15;

    if (g_GF_SunsetEnabled) {
        g_SunsetTimer += ticks;
        CLAMPG(g_SunsetTimer, SUNSET_TIMEOUT);
        g_RoomLightShades[3] =
            g_SunsetTimer * (WIBBLE_SIZE - 1) / SUNSET_TIMEOUT;
    }

    Output_DoAnimateTextures(ticks);
}

void Output_AddDynamicLight(
    const int32_t x, const int32_t y, const int32_t z, const int32_t intensity,
    const int32_t falloff)
{
    const int32_t idx =
        g_DynamicLightCount < MAX_DYNAMIC_LIGHTS ? g_DynamicLightCount++ : 0;

    LIGHT *const light = &g_DynamicLights[idx];
    light->pos.x = x;
    light->pos.y = y;
    light->pos.z = z;
    light->intensity_1 = intensity;
    light->falloff_1 = falloff;
}
