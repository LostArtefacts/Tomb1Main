#include "game/math.h"
#include "utils.h"

uint32_t Math_Sqrt(uint32_t n)
{
    uint32_t result = 0;
    uint32_t base = 0x40000000;
    do {
        do {
            uint32_t based_result = base + result;
            result >>= 1;
            if (based_result > n) {
                break;
            }
            n -= based_result;
            result |= base;

            base >>= 2;
        } while (base);

        base >>= 2;
    } while (base);

    return result;
}

void Math_GetVectorAngles(int32_t x, int32_t y, int32_t z, int16_t *dest)
{
    dest[0] = Math_Atan(z, x);

    while ((int16_t)x != x || (int16_t)y != y || (int16_t)z != z) {
        x >>= 2;
        y >>= 2;
        z >>= 2;
    }

    int16_t pitch = Math_Atan(Math_Sqrt(SQUARE(x) + SQUARE(z)), y);
    if ((y > 0 && pitch > 0) || (y < 0 && pitch < 0)) {
        pitch = -pitch;
    }

    dest[1] = pitch;
}

int32_t Math_AngleInCone(int32_t angle1, int32_t angle2, int32_t cone)
{
    const int32_t diff = ((int)(angle1 - angle2 + DEG_180)) % DEG_360 - DEG_180;
    return ABS(diff) < cone;
}

DIRECTION Math_GetDirection(int16_t angle)
{
    return (uint16_t)(angle + DEG_45) / DEG_90;
}

DIRECTION Math_GetDirectionCone(const int16_t angle, const int16_t cone)
{
    if (angle >= -cone && angle <= cone) {
        return DIR_NORTH;
    } else if (angle >= DEG_90 - cone && angle <= DEG_90 + cone) {
        return DIR_WEST;
    } else if (angle >= DEG_180 - cone || angle <= -DEG_180 + cone) {
        return DIR_SOUTH;
    } else if (angle >= -DEG_90 - cone && angle <= -DEG_90 + cone) {
        return DIR_EAST;
    }
    return DIR_UNKNOWN;
}

int16_t Math_DirectionToAngle(const DIRECTION dir)
{
    switch (dir) {
    case DIR_NORTH:
        return 0;
    case DIR_WEST:
        return DEG_90;
    case DIR_SOUTH:
        return DEG_180;
    case DIR_EAST:
        return -DEG_90;
    default:
        return 0;
    }
}

int32_t Math_AngleMean(int32_t angle1, int32_t angle2, double ratio)
{
    int32_t diff = angle2 - angle1;

    if (diff > DEG_180) {
        diff -= DEG_360;
    } else if (diff < -DEG_180) {
        diff += DEG_360;
    }

    int32_t result = angle1 + diff * ratio;

    result %= DEG_360;
    if (result < 0) {
        result += DEG_360;
    }

    return result;
}

int32_t XYZ_32_GetDistance(const XYZ_32 *const pos1, const XYZ_32 *const pos2)
{
    int32_t x = (pos1->x - pos2->x);
    int32_t y = (pos1->y - pos2->y);
    int32_t z = (pos1->z - pos2->z);

    int32_t scale = 0;
    while ((int16_t)x != x || (int16_t)y != y || (int16_t)z != z) {
        scale++;
        x >>= 1;
        y >>= 1;
        z >>= 1;
    }
    return Math_Sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z)) << scale;
}

int32_t XYZ_32_GetDistance0(const XYZ_32 *const pos)
{
    return Math_Sqrt(SQUARE(pos->x) + SQUARE(pos->y) + SQUARE(pos->z));
}

bool XYZ_32_AreEquivalent(const XYZ_32 *const pos1, const XYZ_32 *const pos2)
{
    return pos1->x == pos2->x && pos1->y == pos2->y && pos1->z == pos2->z;
}
