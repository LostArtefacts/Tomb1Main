#include "game/game_string_table/priv.h"

#include "memory.h"

static void M_FreeTable(GS_TABLE *const gs_table)
{
    if (gs_table == nullptr) {
        return;
    }
    if (gs_table->objects != nullptr) {
        GS_OBJECT_ENTRY *cur = gs_table->objects;
        while (cur->key != nullptr) {
            Memory_FreePointer(&cur->key);
            Memory_FreePointer(&cur->name);
            Memory_FreePointer(&cur->description);
            cur++;
        }
        Memory_Free(gs_table->objects);
        gs_table->objects = nullptr;
    }

    if (gs_table->game_strings != nullptr) {
        GS_GAME_STRING_ENTRY *cur = gs_table->game_strings;
        while (cur->key != nullptr) {
            Memory_FreePointer(&cur->key);
            Memory_FreePointer(&cur->value);
            cur++;
        }
        Memory_Free(gs_table->game_strings);
        gs_table->game_strings = nullptr;
    }
}

static void M_FreeLevelsTable(GS_LEVEL_TABLE *const levels)
{
    if (levels->entries != nullptr) {
        for (int32_t i = 0; i < levels->count; i++) {
            Memory_FreePointer(&levels->entries[i].title);
            M_FreeTable(&levels->entries[i].table);
        }
        Memory_FreePointer(&levels->entries);
    }
    levels->count = 0;
}

void GS_File_Free(GS_FILE *const gs_file)
{
    if (gs_file == nullptr) {
        return;
    }
    M_FreeTable(&gs_file->global);
    for (int32_t i = 0; i < GFLT_NUMBER_OF; i++) {
        M_FreeLevelsTable(&gs_file->level_tables[i]);
    }
}
