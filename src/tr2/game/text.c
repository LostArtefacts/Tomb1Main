#include "game/text.h"

#include "decomp/decomp.h"
#include "game/output.h"
#include "game/scaler.h"
#include "global/vars.h"

#include <libtrx/utils.h>

static int32_t M_Scale(const int32_t value);

static int32_t M_Scale(const int32_t value)
{
    return Scaler_Calc(value, SCALER_TARGET_TEXT);
}

void Text_DrawBorder(
    const int32_t x, const int32_t y, const int32_t z, const int32_t width,
    const int32_t height)
{
    const int32_t mesh_idx = g_Objects[O_TEXT_BOX].mesh_idx;

    const int32_t offset = 4;
    const int32_t x0 = x + offset;
    const int32_t y0 = y + offset;
    const int32_t x1 = x0 + width - offset * 2;
    const int32_t y1 = y0 + height - offset * 2;
    const int32_t scale_h = TEXT_BASE_SCALE;
    const int32_t scale_v = TEXT_BASE_SCALE;

    Output_DrawScreenSprite2D(
        x0, y0, z, scale_h, scale_v, mesh_idx + 0, 0x1000, 0);
    Output_DrawScreenSprite2D(
        x1, y0, z, scale_h, scale_v, mesh_idx + 1, 0x1000, 0);
    Output_DrawScreenSprite2D(
        x1, y1, z, scale_h, scale_v, mesh_idx + 2, 0x1000, 0);
    Output_DrawScreenSprite2D(
        x0, y1, z, scale_h, scale_v, mesh_idx + 3, 0x1000, 0);

    int32_t w = (width - offset * 2) * TEXT_BASE_SCALE / 8;
    int32_t h = (height - offset * 2) * TEXT_BASE_SCALE / 8;

    Output_DrawScreenSprite2D(x0, y0, z, w, scale_v, mesh_idx + 4, 0x1000, 0);
    Output_DrawScreenSprite2D(x1, y0, z, scale_h, h, mesh_idx + 5, 0x1000, 0);
    Output_DrawScreenSprite2D(x0, y1, z, w, scale_v, mesh_idx + 6, 0x1000, 0);
    Output_DrawScreenSprite2D(x0, y0, z, scale_h, h, mesh_idx + 7, 0x1000, 0);
}

void Text_DrawText(TEXTSTRING *const text)
{
    if (text->flags.drawn) {
        return;
    }
    text->flags.drawn = 1;

    int32_t box_w = 0;
    int32_t box_h = 0;
    const int32_t scale_h = M_Scale(text->scale.h);
    const int32_t scale_v = M_Scale(text->scale.v);

    if (text->flags.flash) {
        text->flash.count -= g_Camera.num_frames;
        if (text->flash.count <= -text->flash.rate) {
            text->flash.count = text->flash.rate;
        } else if (text->flash.count < 0) {
            return;
        }
    }

    int32_t x = (text->pos.x * M_Scale(TEXT_BASE_SCALE)) / TEXT_BASE_SCALE;
    int32_t y = (text->pos.y * M_Scale(TEXT_BASE_SCALE)) / TEXT_BASE_SCALE;
    int32_t z = text->pos.z;
    int32_t text_width =
        Text_GetWidth(text) * M_Scale(TEXT_BASE_SCALE) / TEXT_BASE_SCALE;

    if (text->flags.centre_h) {
        x += (g_PhdWinWidth - text_width) / 2;
    } else if (text->flags.right) {
        x += g_PhdWinWidth - text_width;
    }

    if (text->flags.centre_v) {
        y += g_PhdWinHeight / 2;
    } else if (text->flags.bottom) {
        y += g_PhdWinHeight;
    }

    int32_t box_x = x
        + (text->background.offset.x * M_Scale(TEXT_BASE_SCALE))
            / TEXT_BASE_SCALE
        - ((2 * scale_h) / TEXT_BASE_SCALE);
    int32_t box_y = y
        + (text->background.offset.y * M_Scale(TEXT_BASE_SCALE))
            / TEXT_BASE_SCALE
        - ((4 * scale_v) / TEXT_BASE_SCALE)
        - ((11 * scale_v) / TEXT_BASE_SCALE);
    const int32_t start_x = x;

    const GLYPH_INFO **glyph_ptr = text->glyphs;
    while (*glyph_ptr != NULL) {
        if (text->flags.multiline && (*glyph_ptr)->role == GLYPH_NEWLINE) {
            y += TEXT_HEIGHT * M_Scale(text->scale.v) / TEXT_BASE_SCALE;
            x = start_x;
            glyph_ptr++;
            continue;
        }

        if ((*glyph_ptr)->role == GLYPH_SPACE) {
            x += text->word_spacing * scale_h / TEXT_BASE_SCALE;
            glyph_ptr++;
            continue;
        }

        if ((*glyph_ptr)->role == GLYPH_SECRET) {
            Output_DrawPickup(
                x + 10, y, 7144,
                g_Objects[O_SECRET_1 + (*glyph_ptr)->mesh_idx].mesh_idx, 4096);
            x += (*glyph_ptr)->width * scale_h / TEXT_BASE_SCALE;
            glyph_ptr++;
            continue;
        }

        if (x >= 0 && x < g_PhdWinWidth && y >= 0 && y < g_PhdWinHeight) {
            Output_DrawScreenSprite2D(
                x, y, z, scale_h, scale_v,
                g_Objects[O_ALPHABET].mesh_idx + (*glyph_ptr)->mesh_idx, 4096,
                0);
        }

        if ((*glyph_ptr)->role != GLYPH_COMBINING) {
            const int32_t spacing = text->letter_spacing + (*glyph_ptr)->width;
            x += spacing * scale_h / TEXT_BASE_SCALE;
        }
        glyph_ptr++;
    }

    if (text->flags.outline || text->flags.background) {
        if (text->background.size.x) {
            const int32_t background_width =
                (text->background.size.x * scale_h) / TEXT_BASE_SCALE;
            box_x += (text_width - background_width) / 2;
            box_w = background_width + 4;
        } else {
            box_w = text_width + 4;
        }

        const int32_t background_height =
            (text->background.size.y * scale_v) / TEXT_BASE_SCALE;
        box_h = text->background.size.y ? background_height
                                        : ((16 * scale_v) / TEXT_BASE_SCALE);
    }

    if (text->flags.background) {
        Output_DrawScreenFBox(
            box_x, box_y, z + text->background.offset.z, box_w, box_h, 0, NULL,
            0);
    }

    if (text->flags.outline) {
        Text_DrawBorder(box_x, box_y, z, box_w, box_h);
    }
}

int32_t Text_GetMaxLineLength(void)
{
    return 640 / (TEXT_HEIGHT * 0.75);
}
