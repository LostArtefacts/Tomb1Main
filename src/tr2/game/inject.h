#pragma once

#include "global/types.h"

typedef enum {
    IDT_FLOOR_EDIT = 0,
    IDT_ITEM_EDIT = 1,
    IDT_NUMBER_OF = 2,
} INJECTION_DATA_TYPE;

int32_t Inject_GetDataCount(INJECTION_DATA_TYPE type);
void Inject_Init(int32_t injection_count, char *filenames[]);
void Inject_AllInjections(void);
void Inject_Cleanup();
