#ifndef T1M_GAME_AI_MUMMY_H
#define T1M_GAME_AI_MUMMY_H

#include "global/types.h"

#include <stdint.h>

#define MUMMY_HITPOINTS 18

typedef enum {
    MUMMY_EMPTY = 0,
    MUMMY_STOP = 1,
    MUMMY_DEATH = 2,
} MUMMY_ANIM;

void SetupMummy(OBJECT_INFO *obj);
void InitialiseMummy(int16_t item_num);
void MummyControl(int16_t item_num);

#endif
