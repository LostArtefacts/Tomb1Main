#pragma once

#include "./base.h"

typedef struct {
    bool is_selectable;
    int32_t visible_rows;
    int32_t width;
    int32_t row_height;
} UI_REQUESTER_SETTINGS;

UI_WIDGET *UI_Requester_Create(UI_REQUESTER_SETTINGS settings);
int32_t UI_Requester_GetSelectedRow(UI_WIDGET *requester);
void UI_Requester_ClearRows(UI_WIDGET *requester);
void UI_Requester_SetTitle(UI_WIDGET *requester, const char *title);
void UI_Requester_AddRowLR(
    UI_WIDGET *requester, const char *text_l, const char *text_r,
    void *user_data);
void UI_Requester_AddRowC(
    UI_WIDGET *requester, const char *text, void *user_data);
void *UI_Requester_GetRowUserData(UI_WIDGET *widget, int32_t idx);
int32_t UI_Requester_GetRowCount(UI_WIDGET *widget);
void UI_Requester_ChangeRowLR(
    UI_WIDGET *widget, int32_t idx, const char *text_l, const char *text_r,
    void *user_data);
