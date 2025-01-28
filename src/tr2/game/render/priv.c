#include "game/render/priv.h"

#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/utils.h>

bool g_DiscardTransparent = false;

static void M_QuickSort(int32_t left, int32_t right);
static inline void M_ClipG(
    VERTEX_INFO *buf, const VERTEX_INFO *vtx1, const VERTEX_INFO *vtx2,
    float clip);
static inline void M_ClipGUV(
    VERTEX_INFO *buf, const VERTEX_INFO *vtx1, const VERTEX_INFO *vtx2,
    float clip);

static void M_QuickSort(const int32_t left, const int32_t right)
{
    const int32_t compare = g_SortBuffer[(left + right) / 2]._1;
    int32_t i = left;
    int32_t j = right;

    do {
        while ((i < right) && (g_SortBuffer[i]._1 > compare)) {
            i++;
        }
        while ((left < j) && (compare > g_SortBuffer[j]._1)) {
            j--;
        }
        if (i > j) {
            break;
        }

        SORT_ITEM tmp_item;
        SWAP(g_SortBuffer[i], g_SortBuffer[j], tmp_item);

        i++;
        j--;
    } while (i <= j);

    if (left < j) {
        M_QuickSort(left, j);
    }
    if (i < right) {
        M_QuickSort(i, right);
    }
}

static inline void M_ClipG(
    VERTEX_INFO *const buf, const VERTEX_INFO *const vtx1,
    const VERTEX_INFO *const vtx2, const float clip)
{
    buf->rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
    buf->g = vtx2->g + (vtx1->g - vtx2->g) * clip;
}

static inline void M_ClipGUV(
    VERTEX_INFO *const buf, const VERTEX_INFO *const vtx1,
    const VERTEX_INFO *const vtx2, const float clip)
{
    buf->rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
    buf->g = vtx2->g + (vtx1->g - vtx2->g) * clip;
    buf->u = vtx2->u + (vtx1->u - vtx2->u) * clip;
    buf->v = vtx2->v + (vtx1->v - vtx2->v) * clip;
}

double Render_CalculatePolyZ(
    const SORT_TYPE sort_type, const double z0, const double z1,
    const double z2, const double z3)
{
    double zv = 0.0;
    switch (sort_type) {
    case ST_AVG_Z:
        zv = (z3 > 0.0) ? (z0 + z1 + z2 + z3) / 4.0 : (z0 + z1 + z2) / 3.0;
        break;

    case ST_MAX_Z:
        zv = z0;
        CLAMPL(zv, z1);
        CLAMPL(zv, z2);
        if (z3 > 0.0) {
            CLAMPL(zv, z3);
        }
        break;

    case ST_FAR_Z:
    default:
        zv = 1000000000.0;
        break;
    }
    return zv;
}

void Render_SortPolyList(void)
{
    if (g_SurfaceCount) {
        for (int32_t i = 0; i < g_SurfaceCount; i++) {
            g_SortBuffer[i]._1 += i;
        }
        M_QuickSort(0, g_SurfaceCount - 1);
    }
}

int32_t Render_GetUVAdjustment(void)
{
    if (g_Config.rendering.render_mode == RM_HARDWARE
        && (g_Config.rendering.texel_adjust_mode == TAM_ALWAYS
            || (g_Config.rendering.texel_adjust_mode == TAM_BILINEAR_ONLY
                && g_Config.rendering.texture_filter == GFX_TF_BILINEAR))) {
        return g_Config.rendering.linear_adjustment;
    }

    return g_Config.rendering.nearest_adjustment;
}

void Render_AdjustTextureUVs(const bool reset_uv_add)
{
    if (g_ObjectTextureCount <= 0) {
        return;
    }

    const int32_t offset = Render_GetUVAdjustment();
    for (int32_t i = 0; i < g_ObjectTextureCount; i++) {
        OBJECT_TEXTURE *const texture = Output_GetObjectTexture(i);
        TEXTURE_UV *const uv = texture->uv;
        const TEXTURE_UV *const uv_backup = texture->uv_backup;
        int32_t uv_flags = g_LabTextureUVFlag[i];
        for (int32_t j = 0; j < 4; j++) {
            uv[j].u = uv_backup[j].u + ((uv_flags & 1) ? -offset : offset);
            uv[j].v = uv_backup[j].v + ((uv_flags & 2) ? -offset : offset);
            uv_flags >>= 2;
        }
    }
}

int32_t Render_VisibleZClip(
    const PHD_VBUF *const vtx0, const PHD_VBUF *const vtx1,
    const PHD_VBUF *const vtx2)
{
    // clang-format off
    return (
        vtx1->xv * (vtx0->yv * vtx2->zv - vtx0->zv * vtx2->yv) +
        vtx1->yv * (vtx0->zv * vtx2->xv - vtx0->xv * vtx2->zv) +
        vtx1->zv * (vtx0->xv * vtx2->yv - vtx0->yv * vtx2->xv) < 0.0
    );
    // clang-format on
}

int32_t Render_ZedClipper(
    const int32_t vtx_count, const POINT_INFO *const points,
    VERTEX_INFO *const vtx)
{
    int32_t j = 0;
    const POINT_INFO *pts0 = &points[0];
    const POINT_INFO *pts1 = &points[vtx_count - 1];

    for (int32_t i = 0; i < vtx_count; i++) {
        const int32_t diff0 = g_FltNearZ - pts0->zv;
        const int32_t diff1 = g_FltNearZ - pts1->zv;
        if ((diff0 | diff1) >= 0) {
            goto loop_end;
        }

        if ((diff0 ^ diff1) < 0) {
            const double clip = diff0 / (pts1->zv - pts0->zv);
            vtx[j].x =
                (pts0->xv + (pts1->xv - pts0->xv) * clip) * g_FltPerspONearZ
                + g_FltWinCenterX;
            vtx[j].y =
                (pts0->yv + (pts1->yv - pts0->yv) * clip) * g_FltPerspONearZ
                + g_FltWinCenterY;
            vtx[j].z = pts0->zv + (pts1->zv - pts0->zv) * clip;
            vtx[j].rhw = g_FltRhwONearZ;
            vtx[j].g = pts0->g + (pts1->g - pts0->g) * clip;
            vtx[j].u = (pts0->u + (pts1->u - pts0->u) * clip) * g_FltRhwONearZ;
            vtx[j].v = (pts0->v + (pts1->v - pts0->v) * clip) * g_FltRhwONearZ;
            j++;
        }

        if (diff0 < 0) {
            vtx[j].x = pts0->xs;
            vtx[j].y = pts0->ys;
            vtx[j].z = pts0->zv;
            vtx[j].rhw = pts0->rhw;
            vtx[j].g = pts0->g;
            vtx[j].u = pts0->u * pts0->rhw;
            vtx[j].v = pts0->v * pts0->rhw;
            j++;
        }

    loop_end:
        pts1 = pts0++;
    }

    return (j < 3) ? 0 : j;
}

int32_t Render_XYClipper(int32_t vtx_count, VERTEX_INFO *const vtx)
{
    int32_t j;
    VERTEX_INFO vtx_buf[20];
    const VERTEX_INFO *vtx1;
    const VERTEX_INFO *vtx2;

    if (vtx_count < 3) {
        return 0;
    }

    // horizontal clip
    j = 0;
    vtx2 = &vtx[vtx_count - 1];
    for (int32_t i = 0; i < vtx_count; i++) {
        vtx1 = vtx2;
        vtx2 = &vtx[i];

        if (vtx1->x < g_FltWinLeft) {
            if (vtx2->x < g_FltWinLeft) {
                continue;
            }
            const float clip = (g_FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinLeft;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            vtx_buf[j].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
            j++;
        } else if (vtx1->x > g_FltWinRight) {
            if (vtx2->x > g_FltWinRight) {
                continue;
            }
            const float clip = (g_FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinRight;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            vtx_buf[j].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
            j++;
        }

        if (vtx2->x < g_FltWinLeft) {
            const float clip = (g_FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinLeft;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            vtx_buf[j].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
            j++;
        } else if (vtx2->x > g_FltWinRight) {
            const float clip = (g_FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinRight;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            vtx_buf[j].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
            j++;
        } else {
            vtx_buf[j].x = vtx2->x;
            vtx_buf[j].y = vtx2->y;
            vtx_buf[j].z = vtx2->z;
            vtx_buf[j].rhw = vtx2->rhw;
            j++;
        }
    }

    vtx_count = j;
    if (vtx_count < 3) {
        return 0;
    }

    // vertical clip
    j = 0;
    vtx2 = &vtx_buf[vtx_count - 1];
    for (int32_t i = 0; i < vtx_count; i++) {
        vtx1 = vtx2;
        vtx2 = &vtx_buf[i];

        if (vtx1->y < g_FltWinTop) {
            if (vtx2->y < g_FltWinTop) {
                continue;
            }
            const float clip = (g_FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinTop;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            vtx[j].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
            j++;
        } else if (vtx1->y > g_FltWinBottom) {
            if (vtx2->y > g_FltWinBottom) {
                continue;
            }
            const float clip = (g_FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinBottom;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            vtx[j].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
            j++;
        }

        if (vtx2->y < g_FltWinTop) {
            const float clip = (g_FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinTop;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            vtx[j].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
            j++;
        } else if (vtx2->y > g_FltWinBottom) {
            const float clip = (g_FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinBottom;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            vtx[j].rhw = vtx2->rhw + (vtx1->rhw - vtx2->rhw) * clip;
            j++;
        } else {
            vtx[j].x = vtx2->x;
            vtx[j].y = vtx2->y;
            vtx[j].z = vtx2->z;
            vtx[j].rhw = vtx2->rhw;
            j++;
        }
    }

    return (j < 3) ? 0 : j;
}

int32_t Render_XYGClipper(int32_t vtx_count, VERTEX_INFO *const vtx)
{
    VERTEX_INFO vtx_buf[8];
    const VERTEX_INFO *vtx1;
    const VERTEX_INFO *vtx2;
    int32_t j;

    if (vtx_count < 3) {
        return 0;
    }

    // horizontal clip
    j = 0;
    vtx2 = &vtx[vtx_count - 1];
    for (int32_t i = 0; i < vtx_count; i++) {
        vtx1 = vtx2;
        vtx2 = &vtx[i];

        if (vtx1->x < g_FltWinLeft) {
            if (vtx2->x < g_FltWinLeft) {
                continue;
            }
            const float clip = (g_FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinLeft;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipG(&vtx_buf[j++], vtx1, vtx2, clip);
        } else if (vtx1->x > g_FltWinRight) {
            if (vtx2->x > g_FltWinRight) {
                continue;
            }
            const float clip = (g_FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinRight;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipG(&vtx_buf[j++], vtx1, vtx2, clip);
        }

        if (vtx2->x < g_FltWinLeft) {
            const float clip = (g_FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinLeft;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipG(&vtx_buf[j++], vtx1, vtx2, clip);
        } else if (vtx2->x > g_FltWinRight) {
            const float clip = (g_FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinRight;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipG(&vtx_buf[j++], vtx1, vtx2, clip);
        } else {
            vtx_buf[j++] = *vtx2;
        }
    }

    vtx_count = j;
    if (vtx_count < 3) {
        return 0;
    }

    // vertical clip
    j = 0;
    vtx2 = &vtx_buf[vtx_count - 1];
    for (int32_t i = 0; i < vtx_count; i++) {
        vtx1 = vtx2;
        vtx2 = &vtx_buf[i];

        if (vtx1->y < g_FltWinTop) {
            if (vtx2->y < g_FltWinTop) {
                continue;
            }
            const float clip = (g_FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinTop;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipG(&vtx[j++], vtx1, vtx2, clip);
        } else if (vtx1->y > g_FltWinBottom) {
            if (vtx2->y > g_FltWinBottom) {
                continue;
            }
            const float clip = (g_FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinBottom;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipG(&vtx[j++], vtx1, vtx2, clip);
        }

        if (vtx2->y < g_FltWinTop) {
            const float clip = (g_FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinTop;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipG(&vtx[j++], vtx1, vtx2, clip);
        } else if (vtx2->y > g_FltWinBottom) {
            const float clip = (g_FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinBottom;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipG(&vtx[j++], vtx1, vtx2, clip);
        } else {
            vtx[j++] = *vtx2;
        }
    }

    return (j < 3) ? 0 : j;
}

int32_t Render_XYGUVClipper(int32_t vtx_count, VERTEX_INFO *const vtx)
{
    VERTEX_INFO vtx_buf[8];
    const VERTEX_INFO *vtx1;
    const VERTEX_INFO *vtx2;
    int32_t j;

    if (vtx_count < 3) {
        return 0;
    }

    // horizontal clip
    j = 0;
    vtx2 = &vtx[vtx_count - 1];
    for (int32_t i = 0; i < vtx_count; i++) {
        vtx1 = vtx2;
        vtx2 = &vtx[i];

        if (vtx1->x < g_FltWinLeft) {
            if (vtx2->x < g_FltWinLeft) {
                continue;
            }
            float clip = (g_FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinLeft;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipGUV(&vtx_buf[j++], vtx1, vtx2, clip);
        } else if (vtx1->x > g_FltWinRight) {
            if (vtx2->x > g_FltWinRight) {
                continue;
            }
            float clip = (g_FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinRight;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipGUV(&vtx_buf[j++], vtx1, vtx2, clip);
        }

        if (vtx2->x < g_FltWinLeft) {
            float clip = (g_FltWinLeft - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinLeft;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipGUV(&vtx_buf[j++], vtx1, vtx2, clip);
        } else if (vtx2->x > g_FltWinRight) {
            float clip = (g_FltWinRight - vtx2->x) / (vtx1->x - vtx2->x);
            vtx_buf[j].x = g_FltWinRight;
            vtx_buf[j].y = vtx2->y + (vtx1->y - vtx2->y) * clip;
            vtx_buf[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipGUV(&vtx_buf[j++], vtx1, vtx2, clip);
        } else {
            vtx_buf[j++] = *vtx2;
        }
    }

    vtx_count = j;
    if (vtx_count < 3) {
        return 0;
    }

    // vertical clip
    j = 0;
    vtx2 = &vtx_buf[vtx_count - 1];
    for (int32_t i = 0; i < vtx_count; i++) {
        vtx1 = vtx2;
        vtx2 = &vtx_buf[i];

        if (vtx1->y < g_FltWinTop) {
            if (vtx2->y < g_FltWinTop) {
                continue;
            }
            const float clip = (g_FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinTop;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipGUV(&vtx[j++], vtx1, vtx2, clip);
        } else if (vtx1->y > g_FltWinBottom) {
            if (vtx2->y > g_FltWinBottom) {
                continue;
            }
            const float clip = (g_FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinBottom;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipGUV(&vtx[j++], vtx1, vtx2, clip);
        }

        if (vtx2->y < g_FltWinTop) {
            const float clip = (g_FltWinTop - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinTop;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipGUV(&vtx[j++], vtx1, vtx2, clip);
        } else if (vtx2->y > g_FltWinBottom) {
            const float clip = (g_FltWinBottom - vtx2->y) / (vtx1->y - vtx2->y);
            vtx[j].x = vtx2->x + (vtx1->x - vtx2->x) * clip;
            vtx[j].y = g_FltWinBottom;
            vtx[j].z = vtx2->z + (vtx1->z - vtx2->z) * clip;
            M_ClipGUV(&vtx[j++], vtx1, vtx2, clip);
        } else {
            vtx[j++] = *vtx2;
        }
    }

    return (j < 3) ? 0 : j;
}
