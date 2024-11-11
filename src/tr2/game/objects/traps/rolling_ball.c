#include "game/objects/traps/rolling_ball.h"

#include "game/gamebuf.h"
#include "game/items.h"
#include "game/math.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

#define ROLLING_BALL_DAMAGE_AIR 100
#define ROLL_SHAKE_RANGE (WALL_L * 10) // = 10240

void __cdecl RollingBall_Initialise(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    GAME_VECTOR *const data =
        GameBuf_Alloc(sizeof(GAME_VECTOR), GBUF_ROLLING_BALL_STUFF);
    data->pos.x = item->pos.x;
    data->pos.y = item->pos.y;
    data->pos.z = item->pos.z;
    data->room_num = item->room_num;

    item->data = data;
}
