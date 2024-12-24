#include "game/objects/effects/body_part.h"

#include "game/effects.h"
#include "game/lara/control.h"
#include "game/lara/misc.h"
#include "game/math.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/types.h"
#include "global/vars.h"

void BodyPart_Setup(void)
{
    OBJECT *const obj = Object_GetObject(O_BODY_PART);
    obj->control = BodyPart_Control;
    obj->loaded = 1;
    obj->mesh_count = 0;
}

void __cdecl BodyPart_Control(const int16_t effect_num)
{
    EFFECT *const effect = &g_Effects[effect_num];
    effect->rot.x += 5 * PHD_DEGREE;
    effect->rot.z += 10 * PHD_DEGREE;
    effect->pos.x += (effect->speed * Math_Sin(effect->rot.y)) >> W2V_SHIFT;
    effect->pos.z += (effect->speed * Math_Cos(effect->rot.y)) >> W2V_SHIFT;
    effect->pos.y += effect->fall_speed;
    effect->fall_speed += GRAVITY;

    int16_t room_num = effect->room_num;
    const SECTOR *const sector =
        Room_GetSector(effect->pos.x, effect->pos.y, effect->pos.z, &room_num);

    if (!(g_Rooms[effect->room_num].flags & RF_UNDERWATER)
        && (g_Rooms[room_num].flags & RF_UNDERWATER)) {
        const int16_t effect_num = Effect_Create(effect->room_num);
        if (effect_num != NO_EFFECT) {
            EFFECT *const splash_fx = &g_Effects[effect_num];
            splash_fx->pos.x = effect->pos.x;
            splash_fx->pos.y = effect->pos.y;
            splash_fx->pos.z = effect->pos.z;
            splash_fx->rot.y = 0;
            splash_fx->speed = 0;
            splash_fx->frame_num = 0;
            splash_fx->object_id = O_SPLASH;
        }
    }

    const int32_t ceiling =
        Room_GetCeiling(sector, effect->pos.x, effect->pos.y, effect->pos.z);
    if (effect->pos.y < ceiling) {
        effect->pos.y = ceiling;
        effect->fall_speed = -effect->fall_speed;
    }

    const int32_t height =
        Room_GetHeight(sector, effect->pos.x, effect->pos.y, effect->pos.z);
    if (effect->pos.y >= height) {
        if (effect->counter) {
            effect->speed = 0;
            effect->frame_num = 0;
            effect->counter = 0;
            effect->object_id = O_EXPLOSION;
            effect->shade = HIGH_LIGHT;
            Sound_Effect(SFX_EXPLOSION_1, &effect->pos, SPM_NORMAL);
        } else {
            Effect_Kill(effect_num);
        }
        return;
    }

    if (Lara_IsNearItem(&effect->pos, 2 * effect->counter)) {
        Lara_TakeDamage(effect->counter, true);

        if (effect->counter == 0) {
            effect->speed = 0;
            effect->frame_num = 0;
            effect->counter = 0;
            effect->object_id = O_EXPLOSION;
            effect->shade = HIGH_LIGHT;
            Sound_Effect(SFX_EXPLOSION_1, &effect->pos, SPM_NORMAL);
            g_Lara.spaz_effect_count = 5;
            g_Lara.spaz_effect = effect;
        } else {
            Effect_Kill(effect_num);
        }
    }

    if (room_num != effect->room_num) {
        Effect_NewRoom(effect_num, room_num);
    }
}
