#pragma once

#include <stdint.h>

#define TEXT_MAX_STRINGS 128
#define TEXT_BASE_SCALE 0x10000

typedef enum {
    TS_HEADING = 0,
    TS_BACKGROUND = 1,
    TS_REQUESTED = 2,
} TEXT_STYLE;

typedef struct {
    union {
        uint32_t all;
        struct {
            uint32_t active : 1;
            uint32_t flash : 1;
            uint32_t rotate_h : 1;
            uint32_t centre_h : 1;
            uint32_t centre_v : 1;
            uint32_t right : 1;
            uint32_t bottom : 1;
            uint32_t background : 1;
            uint32_t outline : 1;
            uint32_t hide : 1;
            uint32_t multiline : 1;
            uint32_t manual_draw : 1;
        };
    } flags;

    struct {
        int16_t x;
        int16_t y;
        int16_t z;
    } pos;

    int16_t letter_spacing;
    int16_t word_spacing;

    struct {
        int16_t rate;
        int16_t count;
    } flash;

    struct {
        int32_t h;
        int32_t v;
    } scale;

    struct {
        TEXT_STYLE style;
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        } offset;
        struct {
            int16_t x;
            int16_t y;
        } size;
    } background;

    struct {
        TEXT_STYLE style;
    } outline;

    char *content;
} TEXTSTRING;

extern int32_t Text_GetMaxLineLength(void);
extern void Text_DrawText(TEXTSTRING *text);

void Text_Init(void);
void Text_Shutdown(void);

TEXTSTRING *Text_Create(int16_t x, int16_t y, const char *text);
void Text_ChangeText(TEXTSTRING *text, const char *content);
void Text_Draw(void);
