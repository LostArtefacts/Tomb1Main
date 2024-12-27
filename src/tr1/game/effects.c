#include "game/effects.h"

#include "game/output.h"
#include "game/room.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"
#include "math/matrix.h"

#include <libtrx/game/gamebuf.h>

#include <stddef.h>

static EFFECT *m_Effects = NULL;
static int16_t m_NextEffectActive = NO_EFFECT;
static int16_t m_NextEffectFree = NO_EFFECT;

void Effect_InitialiseArray(void)
{
    m_Effects = GameBuf_Alloc(NUM_EFFECTS * sizeof(EFFECT), GBUF_EFFECTS);
    m_NextEffectActive = NO_EFFECT;
    m_NextEffectFree = 0;
    for (int i = 0; i < NUM_EFFECTS - 1; i++) {
        m_Effects[i].next_draw = i + 1;
        m_Effects[i].next_free = i + 1;
    }
    m_Effects[NUM_EFFECTS - 1].next_draw = NO_EFFECT;
    m_Effects[NUM_EFFECTS - 1].next_free = NO_EFFECT;
}

void Effect_Control(void)
{
    int16_t effect_num = m_NextEffectActive;
    while (effect_num != NO_EFFECT) {
        EFFECT *effect = Effect_Get(effect_num);
        OBJECT *obj = &g_Objects[effect->object_id];
        if (obj->control) {
            obj->control(effect_num);
        }
        effect_num = effect->next_active;
    }
}

EFFECT *Effect_Get(const int16_t effect_num)
{
    return &m_Effects[effect_num];
}

int16_t Effect_GetNum(const EFFECT *effect)
{
    return effect - m_Effects;
}

int16_t Effect_GetActiveNum(void)
{
    return m_NextEffectActive;
}

int16_t Effect_Create(int16_t room_num)
{
    int16_t effect_num = m_NextEffectFree;
    if (effect_num == NO_EFFECT) {
        return effect_num;
    }

    EFFECT *effect = Effect_Get(effect_num);
    m_NextEffectFree = effect->next_free;

    ROOM *r = &g_RoomInfo[room_num];
    effect->room_num = room_num;
    effect->next_draw = r->effect_num;
    r->effect_num = effect_num;

    effect->next_active = m_NextEffectActive;
    m_NextEffectActive = effect_num;

    return effect_num;
}

void Effect_Kill(int16_t effect_num)
{
    EFFECT *effect = Effect_Get(effect_num);

    if (m_NextEffectActive == effect_num) {
        m_NextEffectActive = effect->next_active;
    } else {
        int16_t link_num = m_NextEffectActive;
        while (link_num != NO_EFFECT) {
            EFFECT *fx_link = Effect_Get(link_num);
            if (fx_link->next_active == effect_num) {
                fx_link->next_active = effect->next_active;
            }
            link_num = fx_link->next_active;
        }
    }

    ROOM *r = &g_RoomInfo[effect->room_num];
    if (r->effect_num == effect_num) {
        r->effect_num = effect->next_draw;
    } else {
        int16_t link_num = r->effect_num;
        while (link_num != NO_EFFECT) {
            EFFECT *fx_link = Effect_Get(link_num);
            if (fx_link->next_draw == effect_num) {
                fx_link->next_draw = effect->next_draw;
                break;
            }
            link_num = fx_link->next_draw;
        }
    }

    effect->next_free = m_NextEffectFree;
    m_NextEffectFree = effect_num;
}

void Effect_NewRoom(int16_t effect_num, int16_t room_num)
{
    EFFECT *effect = Effect_Get(effect_num);
    ROOM *r = &g_RoomInfo[effect->room_num];

    int16_t link_num = r->effect_num;
    if (link_num == effect_num) {
        r->effect_num = effect->next_draw;
    } else {
        for (; link_num != NO_EFFECT;
             link_num = Effect_Get(link_num)->next_draw) {
            if (Effect_Get(link_num)->next_draw == effect_num) {
                Effect_Get(link_num)->next_draw = effect->next_draw;
                break;
            }
        }
    }

    r = &g_RoomInfo[room_num];
    effect->room_num = room_num;
    effect->next_draw = r->effect_num;
    r->effect_num = effect_num;
}

void Effect_Draw(const int16_t effect_num)
{
    const EFFECT *const effect = Effect_Get(effect_num);
    const OBJECT *const object = &g_Objects[effect->object_id];
    if (!object->loaded) {
        return;
    }

    if (object->nmeshes < 0) {
        Output_DrawSprite(
            effect->interp.result.pos.x, effect->interp.result.pos.y,
            effect->interp.result.pos.z, object->mesh_idx - effect->frame_num,
            4096);
    } else {
        Matrix_Push();
        Matrix_TranslateAbs(
            effect->interp.result.pos.x, effect->interp.result.pos.y,
            effect->interp.result.pos.z);
        if (g_MatrixPtr->_23 > Output_GetNearZ()
            && g_MatrixPtr->_23 < Output_GetFarZ()) {
            Matrix_RotYXZ(
                effect->interp.result.rot.y, effect->interp.result.rot.x,
                effect->interp.result.rot.z);
            if (object->nmeshes) {
                Output_CalculateStaticLight(effect->shade);
                Object_DrawMesh(object->mesh_idx, -1, false);
            } else {
                Output_CalculateLight(
                    effect->interp.result.pos.x, effect->interp.result.pos.y,
                    effect->interp.result.pos.z, effect->room_num);
                Object_DrawMesh(effect->frame_num, -1, false);
            }
        }
        Matrix_Pop();
    }
}
