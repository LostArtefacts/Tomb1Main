#include "game/requester.h"

#include "game/input.h"
#include "game/screen.h"
#include "game/text.h"
#include "global/const.h"
#include "global/types.h"

#include <libtrx/memory.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define BOX_BORDER 2
#define BOX_PADDING 10

void Requester_Init(REQUEST_INFO *req, uint16_t max_items)
{
    req->max_items = max_items;
    req->heading_text = Memory_Alloc(sizeof(char) * req->item_text_len);
    req->item_flags = Memory_Alloc(sizeof(uint32_t) * max_items);
    req->item_texts = Memory_Alloc(sizeof(char *) * max_items);
    for (int i = 0; i < max_items; i++) {
        req->item_texts[i] = Memory_Alloc(sizeof(char) * req->item_text_len);
        req->item_flags[i] = 0;
    }
    Requester_ClearTextstrings(req);
}

void Requester_Shutdown(REQUEST_INFO *req)
{
    Requester_ClearTextstrings(req);

    Memory_FreePointer(&req->heading_text);
    Memory_FreePointer(&req->item_flags);
    for (int i = 0; i < req->max_items; i++) {
        Memory_FreePointer(&req->item_texts[i]);
    }
    Memory_FreePointer(&req->item_texts);
}

void Requester_ClearTextstrings(REQUEST_INFO *req)
{
    Text_Remove(req->heading);
    req->heading = NULL;
    Text_Remove(req->background);
    req->background = NULL;
    Text_Remove(req->moreup);
    req->moreup = NULL;
    Text_Remove(req->moredown);
    req->moredown = NULL;

    for (int i = 0; i < MAX_REQLINES; i++) {
        Text_Remove(req->texts[i]);
        req->texts[i] = NULL;
    }

    req->items_used = 0;
}

int32_t Requester_Display(REQUEST_INFO *req)
{
    int32_t edge_y = req->y;
    int32_t lines_height = req->vis_lines * req->line_height;
    int32_t box_width = req->pix_width;
    int32_t box_height =
        req->line_height + lines_height + BOX_PADDING * 2 + BOX_BORDER * 2;

    int32_t line_one_off = edge_y - lines_height - BOX_PADDING;
    int32_t box_y = line_one_off - req->line_height - BOX_PADDING - BOX_BORDER;
    int32_t line_qty = req->vis_lines;
    if (req->items_used < req->vis_lines) {
        line_qty = req->items_used;
    }

    if (!req->background) {
        req->background = Text_Create(req->x, box_y, " ");
        Text_CentreH(req->background, 1);
        Text_AlignBottom(req->background, 1);
        Text_AddBackground(
            req->background, box_width, box_height, 0, 0, TS_BACKGROUND);
        Text_AddOutline(req->background, true, TS_BACKGROUND);
    }

    if (!req->heading) {
        req->heading = Text_Create(
            req->x, line_one_off - req->line_height - BOX_PADDING,
            req->heading_text);
        Text_CentreH(req->heading, 1);
        Text_AlignBottom(req->heading, 1);
        Text_AddBackground(
            req->heading, req->pix_width - 2 * BOX_BORDER, 0, 0, 0, TS_HEADING);
        Text_AddOutline(req->heading, true, TS_HEADING);
    }

    if (g_InputDB.menu_down) {
        if (req->requested < req->items_used - 1) {
            req->requested++;
        }
        req->line_old_offset = req->line_offset;
        if (req->requested > req->line_offset + req->vis_lines - 1) {
            req->line_offset++;
        }
    }

    if (g_InputDB.menu_up) {
        if (req->requested) {
            req->requested--;
        }
        req->line_old_offset = req->line_offset;
        if (req->requested < req->line_offset) {
            req->line_offset--;
        }
    }

    if (req->line_offset) {
        if (!req->moreup) {
            req->moreup =
                Text_Create(req->x, line_one_off - req->line_height + 2, "[");
            Text_SetScale(req->moreup, PHD_ONE * 2 / 3, PHD_ONE * 2 / 3);
            Text_CentreH(req->moreup, 1);
            Text_AlignBottom(req->moreup, 1);
        }
    } else {
        Text_Remove(req->moreup);
        req->moreup = NULL;
    }

    if (req->items_used > req->vis_lines + req->line_offset) {
        if (!req->moredown) {
            req->moredown = Text_Create(req->x, edge_y - 12, "]");
            Text_SetScale(req->moredown, PHD_ONE * 2 / 3, PHD_ONE * 2 / 3);
            Text_CentreH(req->moredown, 1);
            Text_AlignBottom(req->moredown, 1);
        }
    } else {
        Text_Remove(req->moredown);
        req->moredown = NULL;
    }

    for (int i = 0; i < line_qty; i++) {
        if (!req->texts[i]) {
            req->texts[i] = Text_Create(
                0, line_one_off + req->line_height * i,
                req->item_texts[req->line_offset + i]);
            Text_CentreH(req->texts[i], 1);
            Text_AlignBottom(req->texts[i], 1);
        }
        if (req->line_offset + i == req->requested) {
            Text_AddBackground(
                req->texts[i], req->pix_width - BOX_PADDING - 1 * BOX_BORDER, 0,
                0, 0, TS_REQUESTED);
            Text_AddOutline(req->texts[i], true, TS_REQUESTED);
        } else {
            Text_RemoveBackground(req->texts[i]);
            Text_RemoveOutline(req->texts[i]);
        }
    }

    if (req->line_offset != req->line_old_offset) {
        for (int i = 0; i < line_qty; i++) {
            if (req->texts[i]) {
                Text_ChangeText(
                    req->texts[i], req->item_texts[req->line_offset + i]);
            }
        }
    }

    if (g_InputDB.menu_confirm) {
        if ((req->item_flags[req->requested] & RIF_BLOCKED)
            && (req->flags & RIF_BLOCKABLE)) {
            g_Input = (INPUT_STATE) { 0 };
            return 0;
        } else {
            Requester_ClearTextstrings(req);
            return req->requested + 1;
        }
    } else if (g_InputDB.menu_back) {
        Requester_ClearTextstrings(req);
        return -1;
    }

    return 0;
}

void Requester_SetHeading(REQUEST_INFO *req, const char *string)
{
    Text_Remove(req->heading);
    req->heading = NULL;

    size_t out_size = snprintf(NULL, 0, "%s", string) + 1;
    snprintf(req->heading_text, out_size, "%s", string);
}

void Requester_ChangeItem(
    REQUEST_INFO *req, int32_t idx, const char *string, uint16_t flag)
{
    if (idx > 0 && idx < req->max_items && string) {
        size_t out_size = snprintf(NULL, 0, "%s", string) + 1;
        snprintf(req->item_texts[idx], out_size, "%s", string);
        req->item_flags[idx] = flag;
    }
}

void Requester_AddItem(REQUEST_INFO *req, const char *string, uint16_t flag)
{
    if (req->items_used < req->max_items && string) {
        size_t out_size = snprintf(NULL, 0, "%s", string) + 1;
        snprintf(req->item_texts[req->items_used], out_size, "%s", string);
        req->item_flags[req->items_used] = flag;
        req->items_used++;
    }
}

void Requester_SetSize(REQUEST_INFO *req, int32_t max_lines, int16_t y)
{
    req->y = y;
    req->vis_lines = Screen_GetResHeightDownscaled(RSR_TEXT) / 2 / MAX_REQLINES;
    if (req->vis_lines > max_lines) {
        req->vis_lines = max_lines;
    }
}
