#include "memory.h"

#include "specific/s_init.h"

#include <stdlib.h>
#include <string.h>

void *Memory_Alloc(size_t size)
{
    void *result = malloc(size);
    if (!result) {
        S_ExitSystem("ERROR: Could not allocate enough memory");
    }
    memset(result, 0, size);
    return result;
}

void *Memory_Realloc(void *memory, size_t size)
{
    void *result = realloc(memory, size);
    if (!result) {
        S_ExitSystem("ERROR: Could not allocate enough memory");
    }
    return result;
}

void Memory_Free(void *memory)
{
    if (memory) {
        free(memory);
    }
}
