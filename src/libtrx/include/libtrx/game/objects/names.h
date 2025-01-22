#pragma once

#include "ids.h"

#include <stdint.h>

const char *Object_GetName(GAME_OBJECT_ID object_id);
const char *Object_GetDescription(GAME_OBJECT_ID object_id);

void Object_ResetNames(void);

void Object_SetName(GAME_OBJECT_ID object_id, const char *name);
void Object_SetDescription(GAME_OBJECT_ID object_id, const char *description);

// Return a list of object ids that match given string.
// out_match_count may be NULL.
// The result must be freed by the caller.
GAME_OBJECT_ID *Object_IdsFromName(
    const char *name, int32_t *out_match_count, bool (*filter)(GAME_OBJECT_ID));

// Return an unique object id for a given programmatic string.
// Example:
//     Given a string "key_1", returns O_KEY_1.
GAME_OBJECT_ID Object_IdFromKey(const char *key);
