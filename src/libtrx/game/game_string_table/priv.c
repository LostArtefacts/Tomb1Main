#include "game/game_string_table/priv.h"

#include "memory.h"

void GS_Table_Free(GS_TABLE *const gs_table)
{
    if (gs_table == NULL) {
        return;
    }
    if (gs_table->objects != NULL) {
        GS_OBJECT_ENTRY *cur = gs_table->objects;
        while (cur->key != NULL) {
            Memory_FreePointer(&cur->key);
            Memory_FreePointer(&cur->name);
            Memory_FreePointer(&cur->description);
            cur++;
        }
        Memory_Free(gs_table->objects);
        gs_table->objects = NULL;
    }

    if (gs_table->game_strings != NULL) {
        GS_GAME_STRING_ENTRY *cur = gs_table->game_strings;
        while (cur->key != NULL) {
            Memory_FreePointer(&cur->key);
            Memory_FreePointer(&cur->value);
            cur++;
        }
        Memory_Free(gs_table->game_strings);
        gs_table->game_strings = NULL;
    }
}

void GS_File_Free(GS_FILE *const gs_file)
{
    if (gs_file == NULL) {
        return;
    }
    GS_Table_Free(&gs_file->global);
    if (gs_file->levels != NULL) {
        for (int32_t i = 0; i < gs_file->level_count; i++) {
            Memory_FreePointer(&gs_file->levels[i].title);
            GS_Table_Free(&gs_file->levels[i].table);
        }
    }
    Memory_FreePointer(&gs_file->levels);
    gs_file->level_count = 0;
}
