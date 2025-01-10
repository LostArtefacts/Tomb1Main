#include "game/objects/names.h"

#include "debug.h"
#include "game/game_string.h"
#include "game/objects/common.h"
#include "game/objects/vars.h"
#include "memory.h"
#include "strings/fuzzy_match.h"

typedef struct {
    char *name;
    char *description;
} M_NAME_ENTRY;

static M_NAME_ENTRY m_NamesTable[O_NUMBER_OF] = {};

static void M_ClearNames(void);

static void M_ClearNames(void)
{
    for (GAME_OBJECT_ID object_id = 0; object_id < O_NUMBER_OF; object_id++) {
        M_NAME_ENTRY *const entry = &m_NamesTable[object_id];
        Memory_FreePointer(&entry->name);
        Memory_FreePointer(&entry->description);
    }
}

void Object_SetName(const GAME_OBJECT_ID object_id, const char *const name)
{
    M_NAME_ENTRY *const entry = &m_NamesTable[object_id];
    Memory_FreePointer(&entry->name);
    ASSERT(name != NULL);
    entry->name = Memory_DupStr(name);
}

void Object_SetDescription(
    const GAME_OBJECT_ID object_id, const char *const description)
{
    M_NAME_ENTRY *const entry = &m_NamesTable[object_id];
    Memory_FreePointer(&entry->description);
    ASSERT(description != NULL);
    entry->description = Memory_DupStr(description);
}

const char *Object_GetName(const GAME_OBJECT_ID object_id)
{
    M_NAME_ENTRY *const entry = &m_NamesTable[object_id];
    return entry != NULL ? entry->name : NULL;
}

const char *Object_GetDescription(GAME_OBJECT_ID object_id)
{
    M_NAME_ENTRY *const entry = &m_NamesTable[object_id];
    return entry != NULL ? entry->description : NULL;
}

void Object_ResetNames(void)
{
    M_ClearNames();

#define OBJ_NAME_DEFINE(object_id, name) Object_SetName(object_id, name);
#include "game/objects/names.def"
}

GAME_OBJECT_ID *Object_IdsFromName(
    const char *user_input, int32_t *out_match_count,
    bool (*filter)(GAME_OBJECT_ID))
{
    VECTOR *source = Vector_Create(sizeof(STRING_FUZZY_SOURCE));

    for (GAME_OBJECT_ID object_id = 0; object_id < O_NUMBER_OF; object_id++) {
        if (filter != NULL && !filter(object_id)) {
            continue;
        }

        {
            STRING_FUZZY_SOURCE source_item = {
                .key = Object_GetName(object_id),
                .value = (void *)(intptr_t)object_id,
                .weight = 2,
            };
            Vector_Add(source, &source_item);
        }

        if (Object_IsObjectType(object_id, g_PickupObjects)) {
            STRING_FUZZY_SOURCE source_item = {
                .key = "pickup",
                .value = (void *)(intptr_t)object_id,
                .weight = 1,
            };
            Vector_Add(source, &source_item);
        }
    }

    VECTOR *matches = String_FuzzyMatch(user_input, source);
    GAME_OBJECT_ID *results =
        Memory_Alloc(sizeof(GAME_OBJECT_ID) * (matches->count + 1));
    for (int32_t i = 0; i < matches->count; i++) {
        const STRING_FUZZY_MATCH *const match = Vector_Get(matches, i);
        results[i] = (GAME_OBJECT_ID)(intptr_t)match->value;
    }
    results[matches->count] = NO_OBJECT;
    if (out_match_count != NULL) {
        *out_match_count = matches->count;
    }

    Vector_Free(matches);
    Vector_Free(source);
    matches = NULL;

    return results;
}
