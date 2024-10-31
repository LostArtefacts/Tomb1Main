#include "game/text.h"

#include "memory.h"

#include <assert.h>

static TEXTSTRING m_TextStrings[TEXT_MAX_STRINGS] = { 0 };

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
    }
}

void Text_Draw(void)
{
    for (int i = 0; i < TEXT_MAX_STRINGS; i++) {
        TEXTSTRING *text = &m_TextStrings[i];
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
    text->content = Memory_DupStr(content);
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

    return text;
}

void Text_ChangeText(TEXTSTRING *const text, const char *const content)
{
    if (text == NULL) {
        return;
    }
    assert(content != NULL);
    if (text->flags.active) {
        Memory_FreePointer(&text->content);
        text->content = Memory_DupStr(content);
    }
}

void Text_SetPos(TEXTSTRING *const text, int16_t x, int16_t y)
{
    if (text == NULL) {
        return;
    }
    text->pos.x = x;
    text->pos.y = y;
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
