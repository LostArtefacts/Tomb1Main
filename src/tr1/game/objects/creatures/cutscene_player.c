#include "game/objects/creatures/cutscene_player.h"

#include "game/camera.h"
#include "game/items.h"
#include "global/vars.h"

void CutscenePlayer_Setup(OBJECT *obj)
{
    obj->initialise = CutscenePlayer_Initialise;
    obj->control = CutscenePlayer_Control;
    obj->hit_points = 1;
}

void CutscenePlayer_Initialise(int16_t item_num)
{
    Item_AddActive(item_num);

    ITEM *const item = &g_Items[item_num];
    if (item->object_id == O_PLAYER_1) {
        g_Camera.pos.room_num = item->room_num;
        g_CineData.position.pos = item->pos;
        g_CineData.position.rot.y = 0;
    }
    item->rot.y = 0;
}

void CutscenePlayer_Control(int16_t item_num)
{
    ITEM *const item = &g_Items[item_num];
    if (item->object_id != O_PLAYER_4) {
        item->rot.y = g_CineData.position.rot.y;
        item->pos = g_CineData.position.pos;
    }
    Item_Animate(item);
}
