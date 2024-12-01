#include "memory.h"

#include "debug.h"

#include <stdlib.h>
#include <string.h>

void *Memory_Alloc(const size_t size)
{
    void *result = malloc(size);
    ASSERT(result != NULL);
    memset(result, 0, size);
    return result;
}

void *Memory_Realloc(void *const memory, const size_t size)
{
    void *result = realloc(memory, size);
    ASSERT(result != NULL);
    return result;
}

void Memory_Free(void *const memory)
{
    if (memory != NULL) {
        free(memory);
    }
}

void Memory_FreePointer(void *arg)
{
    ASSERT(arg != NULL);
    void *memory;
    memcpy(&memory, arg, sizeof(void *));
    memcpy(arg, &(void *) { NULL }, sizeof(void *));
    Memory_Free(memory);
}

char *Memory_Dup(const char *const buffer, const size_t size)
{
    ASSERT(buffer != NULL);
    char *memory = Memory_Alloc(size);
    memcpy(memory, buffer, size);
    return memory;
}

char *Memory_DupStr(const char *const string)
{
    ASSERT(string != NULL);
    char *memory = Memory_Alloc(strlen(string) + 1);
    strcpy(memory, string);
    return memory;
}
