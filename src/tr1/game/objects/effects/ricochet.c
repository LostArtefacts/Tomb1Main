#include "game/objects/effects/ricochet.h"

#include "game/effects.h"
#include "game/random.h"
#include "game/sound.h"
#include "global/const.h"

void Ricochet_Setup(OBJECT *obj)
{
    obj->control = Ricochet_Control;
}

void Ricochet_Control(int16_t effect_num)
{
    EFFECT *effect = Effect_Get(effect_num);
    effect->counter--;
    if (!effect->counter) {
        Effect_Kill(effect_num);
    }
}

void Ricochet_Spawn(GAME_VECTOR *pos)
{
    int16_t effect_num = Effect_Create(pos->room_num);
    if (effect_num != NO_EFFECT) {
        EFFECT *effect = Effect_Get(effect_num);
        effect->pos.x = pos->x;
        effect->pos.y = pos->y;
        effect->pos.z = pos->z;
        effect->counter = 4;
        effect->object_id = O_RICOCHET_1;
        effect->frame_num = -3 * Random_GetDraw() / 0x8000;
        Sound_Effect(SFX_LARA_RICOCHET, &effect->pos, SPM_NORMAL);
    }
}
