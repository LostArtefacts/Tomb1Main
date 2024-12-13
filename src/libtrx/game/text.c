#include "game/text.h"

#include "debug.h"
#include "log.h"
#include "memory.h"
#include "utils.h"

#include <string.h>
#include <uthash.h>

typedef struct {
    GLYPH_INFO *glyph;
    UT_hash_handle hh;
} M_HASH_ENTRY;

static TEXTSTRING m_TextStrings[TEXT_MAX_STRINGS] = { 0 };

static GLYPH_INFO m_Glyphs[] = {
#define GLYPH_DEFINE(text_, role_, width_, mesh_idx_, ...)                     \
    { .text = text_,                                                           \
      .role = role_,                                                           \
      .width = width_,                                                         \
      .mesh_idx = mesh_idx_,                                                   \
      __VA_ARGS__ },
#if TR_VERSION == 1
    #include "text_tr1.def"
#elif TR_VERSION == 2
    #include "text_tr2.def"
#endif
    { .text = NULL }, // guard
};

static size_t m_GlyphLookupKeyCap = 0;
static char *m_GlyphLookupKey = NULL;
static M_HASH_ENTRY *m_GlyphMap = NULL;

static size_t M_GetGlyphSize(const char *ptr);

static size_t M_GetGlyphSize(const char *const ptr)
{
    // Check for named escape sequence.
    if (*ptr == '\\' && *(ptr + 1) == '{') {
        const char *end = strchr(ptr + 2, '}');
        if (end != NULL) {
            return end + 1 - ptr;
        }
        return 1;
    }

    // clang-format off
    // UTF-8 sequence lengths
    if ((*ptr & 0x80) == 0x00) { return 1; } // 1-byte sequence
    if ((*ptr & 0xE0) == 0xC0) { return 2; } // 2-byte sequence
    if ((*ptr & 0xF0) == 0xE0) { return 3; } // 3-byte sequence
    if ((*ptr & 0xF8) == 0xF0) { return 4; } // 4-byte sequence
    // clang-format on

    // Fallback to 1
    return 1;
}

void Text_Init(void)
{
    for (int32_t i = 0; i < TEXT_MAX_STRINGS; i++) {
        TEXTSTRING *const text = &m_TextStrings[i];
        text->flags.all = 0;
    }

    // Convert the linear array coming from the .def macros to a hash lookup
    // table for faster text-to-glyph resolution.
    for (GLYPH_INFO *glyph_ptr = m_Glyphs; glyph_ptr->text != NULL;
         glyph_ptr++) {
        M_HASH_ENTRY *const hash_entry = Memory_Alloc(sizeof(M_HASH_ENTRY));
        hash_entry->glyph = glyph_ptr;
        HASH_ADD_KEYPTR(
            hh, m_GlyphMap, glyph_ptr->text, strlen(glyph_ptr->text),
            hash_entry);
    }

    m_GlyphLookupKeyCap = 10;
    m_GlyphLookupKey = Memory_Alloc(m_GlyphLookupKeyCap);
}

void Text_Shutdown(void)
{
    for (int32_t i = 0; i < TEXT_MAX_STRINGS; i++) {
        TEXTSTRING *const text = &m_TextStrings[i];
        Memory_FreePointer(&text->content);
        Memory_FreePointer(&text->glyphs);
    }

    M_HASH_ENTRY *current, *tmp;
    HASH_ITER(hh, m_GlyphMap, current, tmp)
    {
        HASH_DEL(m_GlyphMap, current);
        Memory_Free(current);
    }

    Memory_FreePointer(&m_GlyphLookupKey);
    m_GlyphLookupKeyCap = 0;
}

void Text_DrawReset(void)
{
    for (int32_t i = 0; i < TEXT_MAX_STRINGS; i++) {
        TEXTSTRING *const text = &m_TextStrings[i];
        text->flags.drawn = 0;
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
        Memory_FreePointer(&text->content);
        Memory_FreePointer(&text->glyphs);
    }
}

void Text_ChangeText(TEXTSTRING *const text, const char *const content)
{
    if (text == NULL) {
        return;
    }

    ASSERT(content != NULL);
    Memory_FreePointer(&text->content);
    Memory_FreePointer(&text->glyphs);
    if (!text->flags.active) {
        return;
    }

    // Count number of characters
    size_t glyph_count = 0;
    const char *content_ptr = content;
    while (*content_ptr != '\0') {
        const size_t glyph_size = M_GetGlyphSize(content_ptr);
        content_ptr += glyph_size;
        glyph_count++;
    }

    text->content = Memory_DupStr(content);
    text->glyphs = Memory_Alloc((glyph_count + 1) * sizeof(GLYPH_INFO *));

    // Assign glyphs using hash table
    content_ptr = content;
    const GLYPH_INFO **glyph_ptr = text->glyphs;
    while (*content_ptr != '\0') {
        const size_t glyph_size = M_GetGlyphSize(content_ptr);
        if (m_GlyphLookupKeyCap <= glyph_size) {
            m_GlyphLookupKeyCap = glyph_size + 10;
            m_GlyphLookupKey =
                Memory_Realloc(m_GlyphLookupKey, m_GlyphLookupKeyCap);
        }
        strncpy(m_GlyphLookupKey, content_ptr, glyph_size);
        m_GlyphLookupKey[glyph_size] = '\0';

        M_HASH_ENTRY *entry;
        HASH_FIND_STR(m_GlyphMap, m_GlyphLookupKey, entry);

        if (entry != NULL) {
            *glyph_ptr++ = entry->glyph;
        } else {
            LOG_WARNING("Unknown glyph: %s", m_GlyphLookupKey);
        }

        content_ptr += glyph_size;
    }

    // guard
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

    if (text->glyphs == NULL) {
        return 0;
    }

    int32_t width = 0;
    int32_t max_width = 0;
    const GLYPH_INFO **glyph_ptr = text->glyphs;
    while (*glyph_ptr != NULL) {
        if ((*glyph_ptr)->role == GLYPH_SPACE) {
            width += text->word_spacing;
        } else if ((*glyph_ptr)->role == GLYPH_NEWLINE) {
            max_width = MAX(max_width, width);
            width = 0;
        } else {
            width += (*glyph_ptr)->width + text->letter_spacing;
        }
        glyph_ptr++;
    }

    width = MAX(max_width, width);
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
