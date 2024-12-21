#pragma once

#include <stdint.h>

void Hook_Setup(void);

void __cdecl Hook_Control(int16_t item_num);
