#include "game/text.h"

#include "memory.h"

#include <assert.h>
#include <string.h>

static TEXTSTRING m_TextStrings[TEXT_MAX_STRINGS] = { 0 };

static GLYPH_INFO m_Glyphs[] = {
#define GLYPH_DEFINE(mesh_idx_, text_, width_, role_)                          \
    { .mesh_idx = mesh_idx_, .text = text_, .width = width_, .role = role_ },
#include "text.def"
    { .text = NULL }, // guard
};

static const GLYPH_INFO *M_GetGlyph(const char *ptr);

static const GLYPH_INFO *M_GetGlyph(const char *const ptr)
{
    const GLYPH_INFO *best_match = NULL;
    int32_t best_match_size = 0;
    for (int32_t i = 0; m_Glyphs[i].text != NULL; i++) {
        const int32_t match_size = strlen(m_Glyphs[i].text);
        if (strncmp(ptr, m_Glyphs[i].text, match_size) == 0
            && match_size >= best_match_size) {
            best_match_size = match_size;
            best_match = &m_Glyphs[i];
        }
    }
    return best_match;
}

void Text_Init(void)
{
    for (int32_t i = 0; i < TEXT_MAX_STRINGS; i++) {
        TEXTSTRING *const text = &m_TextStrings[i];
        text->flags.all = 0;
    }
}

void Text_Shutdown(void)
{
    for (int32_t i = 0; i < TEXT_MAX_STRINGS; i++) {
        TEXTSTRING *const text = &m_TextStrings[i];
        Memory_FreePointer(&text->content);
        Memory_FreePointer(&text->glyphs);
    }
}

void Text_Draw(void)
{
    for (int32_t i = 0; i < TEXT_MAX_STRINGS; i++) {
        TEXTSTRING *const text = &m_TextStrings[i];
        if (text->flags.active && !text->flags.manual_draw) {
            Text_DrawText(text);
        }
    }
}

TEXTSTRING *Text_Create(int16_t x, int16_t y, const char *const content)
{
    if (content == NULL) {
        return NULL;
    }

    int32_t free_idx = -1;
    for (int32_t i = 0; i < TEXT_MAX_STRINGS; i++) {
        TEXTSTRING *const text = &m_TextStrings[i];
        if (!text->flags.active) {
            free_idx = i;
            break;
        }
    }

    if (free_idx == -1) {
        return NULL;
    }

    TEXTSTRING *text = &m_TextStrings[free_idx];
    text->content = NULL;
    text->glyphs = NULL;
    text->scale.h = TEXT_BASE_SCALE;
    text->scale.v = TEXT_BASE_SCALE;
    text->pos.x = x;
    text->pos.y = y;
    text->pos.z = 0;
    text->letter_spacing = 1;
    text->word_spacing = 6;

    text->background.size.x = 0;
    text->background.size.y = 0;
    text->background.offset.x = 0;
    text->background.offset.y = 0;
    text->background.offset.z = 0;
    text->flags.all = 0;
    text->flags.active = 1;

    Text_ChangeText(text, content);

    return text;
}

void Text_Remove(TEXTSTRING *const text)
{
    if (text == NULL) {
        return;
    }
    if (text->flags.active) {
        text->flags.active = 0;
    }
}

void Text_ChangeText(TEXTSTRING *const text, const char *const content)
{
    if (text == NULL) {
        return;
    }

    assert(content != NULL);
    Memory_FreePointer(&text->content);
    Memory_FreePointer(&text->glyphs);
    if (!text->flags.active) {
        return;
    }

    text->content = Memory_DupStr(content);
    text->glyphs = Memory_Alloc((strlen(content) + 1) * sizeof(GLYPH_INFO *));

    const char *content_ptr = content;
    const GLYPH_INFO **glyph_ptr = text->glyphs;
    while (*content_ptr != '\0') {
        const GLYPH_INFO *const glyph = M_GetGlyph(content_ptr);
        if (glyph == NULL) {
            content_ptr++;
            continue;
        }

        *glyph_ptr++ = glyph;
        content_ptr += strlen(glyph->text);
    }
    *glyph_ptr++ = NULL;
}

void Text_SetPos(TEXTSTRING *const text, int16_t x, int16_t y)
{
    if (text == NULL) {
        return;
    }
    text->pos.x = x;
    text->pos.y = y;
}

void Text_SetScale(
    TEXTSTRING *const text, const int32_t scale_h, const int32_t scale_v)
{
    if (text == NULL) {
        return;
    }
    text->scale.h = scale_h;
    text->scale.v = scale_v;
}

void Text_Flash(TEXTSTRING *const text, const bool enable, const int16_t rate)
{
    if (text == NULL) {
        return;
    }
    if (enable) {
        text->flags.flash = 1;
        text->flash.rate = rate;
        text->flash.count = rate;
    } else {
        text->flags.flash = 0;
    }
}

void Text_Hide(TEXTSTRING *const text, const bool enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.hide = enable;
}

void Text_AddBackground(
    TEXTSTRING *const text, const int16_t w, const int16_t h, const int16_t x,
    const int16_t y, const TEXT_STYLE style)
{
    if (text == NULL) {
        return;
    }
    text->flags.background = 1;
    text->background.size.x = w;
    text->background.size.y = h;
    text->background.offset.x = x;
    text->background.offset.y = y;
    switch (style) {
    case TS_HEADING:
    case TS_REQUESTED:
        text->background.offset.z = 80;
        break;
    case TS_BACKGROUND:
        text->background.offset.z = 160;
        break;
    }
    text->background.style = style;
}

void Text_RemoveBackground(TEXTSTRING *const text)
{
    if (text == NULL) {
        return;
    }
    text->flags.background = 0;
}

void Text_AddOutline(TEXTSTRING *const text, const TEXT_STYLE style)
{
    if (text == NULL) {
        return;
    }
    text->flags.outline = 1;
    text->outline.style = style;
}

void Text_RemoveOutline(TEXTSTRING *const text)
{
    if (text == NULL) {
        return;
    }
    text->flags.outline = 0;
}

void Text_CentreH(TEXTSTRING *const text, const bool enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.centre_h = enable;
}

void Text_CentreV(TEXTSTRING *const text, const bool enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.centre_v = enable;
}

void Text_AlignRight(TEXTSTRING *const text, const bool enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.right = enable;
}

void Text_AlignBottom(TEXTSTRING *const text, const bool enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.bottom = enable;
}

void Text_SetMultiline(TEXTSTRING *const text, const bool enable)
{
    if (text == NULL) {
        return;
    }
    text->flags.multiline = enable;
}

int32_t Text_GetWidth(const TEXTSTRING *const text)
{
    if (text == NULL) {
        return 0;
    }

    int32_t width = 0;
    const GLYPH_INFO **glyph_ptr = text->glyphs;
    if (text->glyphs == NULL) {
        return 0;
    }
    while (*glyph_ptr != NULL) {
        if ((*glyph_ptr)->role == GLYPH_SPACE) {
            width += text->word_spacing;
        } else {
            width += (*glyph_ptr)->width + text->letter_spacing;
        }
        glyph_ptr++;
    }

    width -= text->letter_spacing;
    return width * text->scale.h / TEXT_BASE_SCALE;
}

int32_t Text_GetHeight(const TEXTSTRING *const text)
{
    if (text == NULL) {
        return 0;
    }
    int32_t height = TEXT_HEIGHT_FIXED;
    char *content = text->content;
    for (char letter = *content; letter != '\0'; letter = *content++) {
        if (text->flags.multiline && letter == '\n') {
            height += TEXT_HEIGHT_FIXED;
        }
    }
    return height * text->scale.v / TEXT_BASE_SCALE;
}
