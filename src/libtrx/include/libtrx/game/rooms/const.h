#pragma once

#define NO_ROOM_NEG (-1)
#define NO_ROOM 255 // TODO: merge this with NO_ROOM_NEG

#define NEG_TILT(T, H) ((T * (H & (WALL_L - 1))) >> 2)
#define POS_TILT(T, H) ((T * ((WALL_L - 1 - H) & (WALL_L - 1))) >> 2)
