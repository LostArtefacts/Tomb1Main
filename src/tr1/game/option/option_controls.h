#pragma once

#include "game/input.h"

#include <libtrx/game/inventory_ring/types.h>

#include <stdbool.h>

typedef enum {
    CM_PICK,
    CM_KEYBOARD,
    CM_CONTROLLER,
} CONTROL_MODE;

CONTROL_MODE Option_Controls_Control(
    INVENTORY_ITEM *inv_item, INPUT_BACKEND backend);
void Option_Controls_Draw(INVENTORY_ITEM *inv_item, INPUT_BACKEND backend);
void Option_Control_Shutdown(void);
