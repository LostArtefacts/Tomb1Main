#pragma once

#include "global/types.h"

typedef struct {
    // TODO: add properties
} INJECTION_INFO;

void Inject_Init(int injection_count, char *filenames[]);
void Inject_AllInjections(void);
void Inject_Cleanup();
