#include "game/text.h"

#include "game/console/common.h"
#include "game/output.h"
#include "game/overlay.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/memory.h>
#include <libtrx/utils.h>

#include <assert.h>
#include <string.h>

#define CHAR_SECRET_1 0x7Fu
#define CHAR_SECRET_2 0x80u
#define CHAR_SECRET_3 0x81u
#define IS_CHAR_LEGAL(c) ((c) <= CHAR_SECRET_3 && ((c) <= 18u || (c) >= 32u))
#define IS_CHAR_SECRET(c) ((c) >= CHAR_SECRET_1 && (c) <= CHAR_SECRET_3)
#define IS_CHAR_DIACRITIC(c)                                                   \
    ((c) == '(' || (c) == ')' || (c) == '$' || (c) == '~')
#define IS_CHAR_SPACE(c) ((c) == 32)
#define IS_CHAR_DIGIT(c) ((c) <= 0xAu)

// TODO: replace textstring == NULL checks with assertions
typedef struct {
    const char *name;
    int32_t sprite_idx;
} M_SPRITE_NAME;

static int32_t M_GetSpriteIndexByName(const char *input, size_t len);

static M_SPRITE_NAME m_SpriteNames[] = {
    { "left", 108 }, { "down", 106 },    { "up", 107 },    { "right", 109 },
    { "x", 95 },     { "triangle", 93 }, { "square", 96 }, { "circle", 94 },
    { "l1", 97 },    { "r1", 98 },       { "l2", 99 },     { "r2", 100 },
    { NULL, -1 },
};

static int32_t M_GetSpriteIndexByName(const char *const input, const size_t len)
{
    for (int32_t i = 0; m_SpriteNames[i].name != NULL; i++) {
        const M_SPRITE_NAME *const def = &m_SpriteNames[i];
        if (strncmp(input, def->name, len) == 0) {
            return def->sprite_idx;
        }
    }
    return -1;
}

void __cdecl Text_RemoveOutline(TEXTSTRING *const text)
{
    if (text == NULL) {
        return;
    }
    text->flags.outline = 0;
}

void __cdecl Text_CentreH(TEXTSTRING *const text, const int16_t enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.centre_h = enable;
}

void __cdecl Text_CentreV(TEXTSTRING *const text, const int16_t enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.centre_v = enable;
}

void __cdecl Text_AlignRight(TEXTSTRING *const text, const int16_t enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.right = enable;
}

void __cdecl Text_AlignBottom(TEXTSTRING *const text, const int16_t enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.bottom = enable;
}

void __cdecl Text_SetMultiline(TEXTSTRING *text, bool enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.multiline = enable;
}

int32_t __cdecl Text_Remove(TEXTSTRING *const text)
{
    if (text == NULL) {
        return false;
    }
    if (!text->flags.active) {
        return false;
    }
    text->flags.active = false;
    return true;
}

int32_t __cdecl Text_GetWidth(TEXTSTRING *const text)
{
    if (text == NULL) {
        return 0;
    }

    const uint32_t scale_h = Text_GetScaleH(text->scale.h);
    const char *content = text->content;
    int32_t width = 0;

    while (1) {
        int32_t spacing = 0;

        const uint8_t c = *content++;
        if (!c) {
            break;
        }

        if (!IS_CHAR_LEGAL(c) || IS_CHAR_DIACRITIC(c)) {
            continue;
        }

        if (IS_CHAR_SPACE(c)) {
            spacing = text->word_spacing;
        } else if (IS_CHAR_SECRET(c)) {
            spacing = 16;
        } else {
            int16_t sprite_num;
            if (c == '\\' && *content == '{') {
                const char *const start = content + 1;
                const char *const end = strchr(start, '}');
                sprite_num = M_GetSpriteIndexByName(content + 1, end - start);
                content = end + 1;
            } else if (IS_CHAR_DIGIT(c)) {
                sprite_num = c + 81;
            } else {
                sprite_num = g_TextASCIIMap[c];
            }

            if (sprite_num == -1) {
                continue;
            }

            // TODO: OG bug - this should check c, not sprite_num
            if (sprite_num >= '0' && sprite_num <= '9') {
                spacing = 12;
            } else {
                // TODO: OG bug - wrong letter spacing calculation
                spacing = g_TextSpacing[sprite_num] + text->letter_spacing;
            }
        }

        width += spacing * scale_h / TEXT_BASE_SCALE;
    }

    // TODO: OG bug - wrong letter spacing calculation; pointless ~1
    return ((int16_t)width - text->letter_spacing) & ~1;
}

int32_t Text_GetHeight(const TEXTSTRING *const text)
{
    if (text == NULL) {
        return 0;
    }
    int32_t height = TEXT_HEIGHT;
    char *content = text->content;
    for (char letter = *content; letter != '\0'; letter = *content++) {
        if (text->flags.multiline && letter == '\n') {
            height += TEXT_HEIGHT;
        }
    }
    return height * Text_GetScaleV(text->scale.v) / TEXT_BASE_SCALE;
}

void __cdecl Text_DrawBorder(
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

void __cdecl Text_DrawText(TEXTSTRING *const text)
{
    int32_t box_w = 0;
    int32_t box_h = 0;
    const int32_t scale_h = Text_GetScaleH(text->scale.h);
    const int32_t scale_v = Text_GetScaleV(text->scale.v);

    if (text->flags.flash) {
        text->flash.count -= g_Camera.num_frames;
        if (text->flash.count <= -text->flash.rate) {
            text->flash.count = text->flash.rate;
        } else if (text->flash.count < 0) {
            return;
        }
    }

    int32_t x =
        (text->pos.x * Text_GetScaleH(TEXT_BASE_SCALE)) / TEXT_BASE_SCALE;
    int32_t y =
        (text->pos.y * Text_GetScaleV(TEXT_BASE_SCALE)) / TEXT_BASE_SCALE;
    int32_t z = text->pos.z;
    int32_t text_width = Text_GetWidth(text);

    if (text->flags.centre_h) {
        x += (GetRenderWidth() - text_width) / 2;
    } else if (text->flags.right) {
        x += GetRenderWidth() - text_width;
    }

    if (text->flags.centre_v) {
        y += GetRenderHeight() / 2;
    } else if (text->flags.bottom) {
        y += GetRenderHeight();
    }

    int32_t box_x = x
        + (text->background.offset.x * Text_GetScaleH(TEXT_BASE_SCALE))
            / TEXT_BASE_SCALE
        - ((2 * scale_h) / TEXT_BASE_SCALE);
    int32_t box_y = y
        + (text->background.offset.y * Text_GetScaleV(TEXT_BASE_SCALE))
            / TEXT_BASE_SCALE
        - ((4 * scale_v) / TEXT_BASE_SCALE)
        - ((11 * scale_v) / TEXT_BASE_SCALE);
    const int32_t start_x = x;

    const char *content = text->content;
    while (1) {
        const uint8_t c = *content++;
        if (!c) {
            break;
        }

        if (text->flags.multiline && c == '\n') {
            y += TEXT_HEIGHT * Text_GetScaleV(text->scale.v) / TEXT_BASE_SCALE;
            x = start_x;
            continue;
        }

        if (!IS_CHAR_LEGAL(c)) {
            continue;
        }

        if (IS_CHAR_SPACE(c)) {
            const int32_t spacing = text->word_spacing;
            x += spacing * scale_h / TEXT_BASE_SCALE;
        } else if (IS_CHAR_SECRET(c)) {
            Output_DrawPickup(
                x + 10, y, 7144,
                g_Objects[O_SECRET_1 + c - CHAR_SECRET_1].mesh_idx, 4096);
            const int32_t spacing = 16;
            x += spacing * scale_h / TEXT_BASE_SCALE;
        } else {
            int16_t sprite_num;

            if (c == '\\' && *content == '{') {
                const char *const start = content + 1;
                const char *const end = strchr(start, '}');
                sprite_num = M_GetSpriteIndexByName(content + 1, end - start);
                content = end + 1;
            } else if (IS_CHAR_DIGIT(c)) {
                sprite_num = c + 81;
            } else if (c <= 0x12) {
                sprite_num = c + 91;
            } else {
                sprite_num = g_TextASCIIMap[c];
            }

            if (sprite_num == -1) {
                continue;
            }

            if (c >= '0' && c <= '9') {
                const int32_t spacing = (12 - g_TextSpacing[sprite_num]) / 2;
                x += spacing * scale_h / TEXT_BASE_SCALE;
            }

            if (x >= 0 && x < GetRenderWidth() && y >= 0
                && y < GetRenderHeight()) {
                Output_DrawScreenSprite2D(
                    x, y, z, scale_h, scale_v,
                    g_Objects[O_ALPHABET].mesh_idx + sprite_num, 4096, 0);
            }

            if (IS_CHAR_DIACRITIC(c)) {
                continue;
            }

            if (c >= '0' && c <= '9') {
                const int32_t x_off = (12 - g_TextSpacing[sprite_num]) / 2;
                x += (12 - x_off) * scale_h / TEXT_BASE_SCALE;
            } else {
                const int32_t spacing =
                    g_TextSpacing[sprite_num] + text->letter_spacing;
                x += spacing * scale_h / TEXT_BASE_SCALE;
            }
        }
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
#if 0
        S_DrawScreenFBox(
            box_x, box_y, text->background.offset.z + z + 2, box_w, box_h,
            text->bgnd_color, (const GOURAUD_FILL *)text->bgnd_gour,
            text->bgnd_flags);
#else
        S_DrawScreenFBox(
            box_x, box_y, text->background.offset.z + z + 2, box_w, box_h, 0,
            NULL, 0);
#endif
    }

    if (text->flags.outline) {
        Text_DrawBorder(box_x, box_y, z, box_w, box_h);
    }
}

int32_t __cdecl Text_GetScaleH(const uint32_t value)
{
    const int32_t render_width = GetRenderWidth();
    const int32_t render_scale = MAX(render_width, 640) * TEXT_BASE_SCALE / 640;
    return (value / PHD_HALF) * (render_scale / PHD_HALF);
}

int32_t __cdecl Text_GetScaleV(const uint32_t value)
{
    const int32_t render_height = GetRenderHeight();
    const int32_t render_scale =
        MAX(render_height, 480) * TEXT_BASE_SCALE / 480;
    return (value / PHD_HALF) * (render_scale / PHD_HALF);
}

int32_t Text_GetMaxLineLength(void)
{
    return 640 / (TEXT_HEIGHT * 0.75);
}
