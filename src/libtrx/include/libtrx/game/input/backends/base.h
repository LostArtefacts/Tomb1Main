#pragma once

#include "../common.h"

#include <stdbool.h>

typedef struct {
    void (*init)(void);
    void (*shutdown)(void);
    bool (*custom_update)(INPUT_STATE *result, INPUT_LAYOUT layout);
    bool (*is_pressed)(INPUT_LAYOUT layout, INPUT_ROLE role);
    bool (*is_role_conflicted)(INPUT_LAYOUT layout, INPUT_ROLE role);
    const char *(*get_name)(INPUT_LAYOUT layout, INPUT_ROLE role);
    void (*unassign_role)(INPUT_LAYOUT layout, INPUT_ROLE role);
    bool (*assign_from_json_object)(
        INPUT_LAYOUT layout, INPUT_ROLE role, JSON_OBJECT *bind_obj);
    bool (*assign_to_json_object)(
        INPUT_LAYOUT layout, INPUT_ROLE role, JSON_OBJECT *bind_obj);
    void (*reset_layout)(INPUT_LAYOUT layout);
    bool (*read_and_assign)(INPUT_LAYOUT layout, INPUT_ROLE role);
} INPUT_BACKEND_IMPL;
