#include "game/effects.h"

#include "game/matrix.h"
#include "game/output.h"
#include "global/const.h"
#include "global/vars.h"

static int16_t m_NextEffectActive;
static int16_t m_NextEffectFree;

static void M_RemoveActive(const int16_t effect_num);
static void M_RemoveDrawn(const int16_t effect_num);

static void M_RemoveActive(const int16_t effect_num)
{
    EFFECT *const effect = Effect_Get(effect_num);
    int16_t link_num = m_NextEffectActive;
    if (link_num == effect_num) {
        m_NextEffectActive = effect->next_active;
        return;
    }

    while (link_num != NO_EFFECT) {
        if (g_Effects[link_num].next_active == effect_num) {
            g_Effects[link_num].next_active = effect->next_active;
            return;
        }
        link_num = g_Effects[link_num].next_active;
    }
}

static void M_RemoveDrawn(const int16_t effect_num)
{
    EFFECT *const effect = Effect_Get(effect_num);
    int16_t link_num = g_Rooms[effect->room_num].effect_num;
    if (link_num == effect_num) {
        g_Rooms[effect->room_num].effect_num = effect->next_free;
        return;
    }

    while (link_num != NO_EFFECT) {
        if (g_Effects[link_num].next_free == effect_num) {
            g_Effects[link_num].next_free = effect->next_free;
            return;
        }
        link_num = g_Effects[link_num].next_free;
    }
}

void __cdecl Effect_InitialiseArray(void)
{
    m_NextEffectFree = 0;
    m_NextEffectActive = NO_EFFECT;

    for (int32_t i = 0; i < MAX_EFFECTS - 1; i++) {
        EFFECT *const effect = Effect_Get(i);
        effect->next_free = i + 1;
    }
    g_Effects[MAX_EFFECTS - 1].next_free = NO_EFFECT;
}

EFFECT *Effect_Get(const int16_t effect_num)
{
    return &g_Effects[effect_num];
}

int16_t Effect_GetActiveNum(void)
{
    return m_NextEffectActive;
}

int16_t __cdecl Effect_Create(const int16_t room_num)
{
    int16_t effect_num = m_NextEffectFree;
    if (effect_num == NO_EFFECT) {
        return NO_EFFECT;
    }

    EFFECT *const effect = Effect_Get(effect_num);
    m_NextEffectFree = effect->next_free;

    ROOM *const room = &g_Rooms[room_num];
    effect->room_num = room_num;
    effect->next_free = room->effect_num;
    room->effect_num = effect_num;

    effect->next_active = m_NextEffectActive;
    m_NextEffectActive = effect_num;

    effect->shade = 0x1000;

    return effect_num;
}

void __cdecl Effect_Kill(const int16_t effect_num)
{
    EFFECT *const effect = Effect_Get(effect_num);
    M_RemoveActive(effect_num);
    M_RemoveDrawn(effect_num);

    effect->next_free = m_NextEffectFree;
    m_NextEffectFree = effect_num;
}

void __cdecl Effect_NewRoom(const int16_t effect_num, const int16_t room_num)
{
    EFFECT *const effect = Effect_Get(effect_num);
    ROOM *room = &g_Rooms[effect->room_num];

    int16_t link_num = room->effect_num;
    if (link_num == effect_num) {
        room->effect_num = effect->next_free;
    } else {
        while (link_num != NO_EFFECT) {
            if (g_Effects[link_num].next_free == effect_num) {
                g_Effects[link_num].next_free = effect->next_free;
                break;
            }
            link_num = g_Effects[link_num].next_free;
        }
    }

    effect->room_num = room_num;
    room = &g_Rooms[room_num];
    effect->next_free = room->effect_num;
    room->effect_num = effect_num;
}

void __cdecl Effect_Draw(const int16_t effect_num)
{
    const EFFECT *const effect = Effect_Get(effect_num);
    const OBJECT *const object = &g_Objects[effect->object_id];
    if (!object->loaded) {
        return;
    }

    if (effect->object_id == O_GLOW) {
        Output_DrawSprite(
            (effect->rot.y << 16) | (unsigned __int16)effect->rot.x,
            effect->pos.x, effect->pos.y, effect->pos.z,
            g_Objects[O_GLOW].mesh_idx, effect->shade, effect->frame_num);
        return;
    }

    if (object->mesh_count < 0) {
        Output_DrawSprite(
            SPRITE_ABS | (object->semi_transparent ? SPRITE_SEMI_TRANS : 0)
                | SPRITE_SHADE,
            effect->pos.x, effect->pos.y, effect->pos.z,
            object->mesh_idx - effect->frame_num, effect->shade, 0);
        return;
    }

    Matrix_Push();
    Matrix_TranslateAbs(effect->pos.x, effect->pos.y, effect->pos.z);
    if (g_MatrixPtr->_23 > g_PhdNearZ && g_MatrixPtr->_23 < g_PhdFarZ) {
        Matrix_RotYXZ(effect->rot.y, effect->rot.x, effect->rot.z);
        if (object->mesh_count) {
            Output_CalculateStaticLight(effect->shade);
            Output_InsertPolygons(g_Meshes[object->mesh_idx], -1);
        } else {
            Output_CalculateStaticLight(effect->shade);
            Output_InsertPolygons(g_Meshes[effect->frame_num], -1);
        }
    }
    Matrix_Pop();
}
