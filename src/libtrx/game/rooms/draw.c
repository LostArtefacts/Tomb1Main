#include "game/rooms.h"

static int32_t m_DrawCount = 0;
static int16_t m_RoomsToDraw[MAX_ROOMS_TO_DRAW] = {};

void Room_DrawReset(void)
{
    m_DrawCount = 0;
}

void Room_MarkToBeDrawn(const int16_t room_num)
{
    if (m_DrawCount + 1 == MAX_ROOMS_TO_DRAW) {
        return;
    }

    for (int32_t i = 0; i < m_DrawCount; i++) {
        if (m_RoomsToDraw[i] == room_num) {
            return;
        }
    }

    m_RoomsToDraw[m_DrawCount++] = room_num;
}

int32_t Room_DrawGetCount(void)
{
    return m_DrawCount;
}

int16_t Room_DrawGetRoom(const int16_t idx)
{
    return m_RoomsToDraw[idx];
}
