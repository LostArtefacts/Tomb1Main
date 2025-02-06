#pragma once

#include <stdint.h>

void Room_DrawReset(void);
void Room_MarkToBeDrawn(int16_t room_num);
int32_t Room_DrawGetCount(void);
int16_t Room_DrawGetRoom(int16_t idx);
