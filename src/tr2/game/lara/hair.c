#include "game/lara/hair.h"

#include "game/items.h"
#include "game/math.h"
#include "game/matrix.h"
#include "game/output.h"
#include "game/random.h"
#include "game/room.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#define HAIR_SEGMENTS 6

typedef struct __PACKING {
    XYZ_32 pos;
    XYZ_16 rot;
} HAIR_SEGMENT;

static bool m_IsFirstHair;
static XYZ_32 m_HairVelocity[HAIR_SEGMENTS + 1];
static HAIR_SEGMENT m_HairSegments[HAIR_SEGMENTS + 1];
static int32_t m_HairWind;

void __cdecl Lara_Hair_Initialise(void)
{
    const int32_t *const bone_base =
        &g_AnimBones[g_Objects[O_LARA_HAIR].bone_idx];
    m_IsFirstHair = true;
    m_HairSegments[0].rot.x = -PHD_90;
    m_HairSegments[0].rot.y = 0;
    for (int32_t i = 0; i < HAIR_SEGMENTS; i++) {
        const int32_t *bone = bone_base + 4 * i;
        m_HairSegments[i + 1].pos.x = bone[1];
        m_HairSegments[i + 1].pos.y = bone[2];
        m_HairSegments[i + 1].pos.z = bone[3];
        m_HairSegments[i + 1].rot.x = -PHD_90;
        m_HairSegments[i + 1].rot.y = 0;
        m_HairSegments[i + 1].rot.z = 0;
        m_HairVelocity[i].x = 0;
        m_HairVelocity[i].y = 0;
        m_HairVelocity[i].z = 0;
    }
}

void __cdecl Lara_Hair_Control(const bool in_cutscene)
{
    const FRAME_INFO *frame;
    if (g_Lara.hit_direction < 0) {
        frame = Item_GetBestFrame(g_LaraItem);
    } else {
        LARA_ANIMATION lara_anim;
        switch (g_Lara.hit_direction) {
        case DIR_EAST:
            lara_anim = LA_HIT_LEFT;
            break;
        case DIR_SOUTH:
            lara_anim = LA_HIT_BACK;
            break;
        case DIR_WEST:
            lara_anim = LA_HIT_RIGHT;
            break;
        default:
            lara_anim = LA_HIT_FRONT;
            break;
        }

        const int16_t *const frame_ptr = g_Anims[lara_anim].frame_ptr;
        const int32_t interpolation = g_Anims[lara_anim].interpolation;
        frame =
            (FRAME_INFO *)&frame_ptr[g_Lara.hit_frame * (interpolation >> 8)];
    }

    const int32_t *bone;
    const int16_t *mesh;
    const int16_t *mesh_rots;
    SPHERE spheres[5];

    Matrix_PushUnit();
    g_MatrixPtr->_03 = g_LaraItem->pos.x << W2V_SHIFT;
    g_MatrixPtr->_13 = g_LaraItem->pos.y << W2V_SHIFT;
    g_MatrixPtr->_23 = g_LaraItem->pos.z << W2V_SHIFT;
    Matrix_RotYXZ(g_LaraItem->rot.y, g_LaraItem->rot.x, g_LaraItem->rot.z);
    mesh_rots = frame->mesh_rots;
    Matrix_TranslateRel(frame->offset.x, frame->offset.y, frame->offset.z);
    Matrix_RotYXZsuperpack(&mesh_rots, 0);
    Matrix_Push();
    mesh = g_Lara.mesh_ptrs[LM_HIPS];
    Matrix_TranslateRel(mesh[0], mesh[1], mesh[2]);
    spheres[0].x = g_MatrixPtr->_03 >> W2V_SHIFT;
    spheres[0].y = g_MatrixPtr->_13 >> W2V_SHIFT;
    spheres[0].z = g_MatrixPtr->_23 >> W2V_SHIFT;
    spheres[0].r = mesh[3];
    Matrix_Pop();

    bone = &g_AnimBones[g_Objects[O_LARA].bone_idx];
    Matrix_TranslateRel(bone[25], bone[26], bone[27]);
    if (g_Lara.weapon_item != NO_ITEM && g_Lara.gun_type == LGT_M16
        && (g_Items[g_Lara.weapon_item].current_anim_state == 0
            || g_Items[g_Lara.weapon_item].current_anim_state == 2
            || g_Items[g_Lara.weapon_item].current_anim_state == 4)) {
        mesh_rots =
            &g_Lara.right_arm.frame_base
                 [g_Lara.right_arm.frame_num
                      * (g_Anims[g_Lara.right_arm.anim_num].interpolation >> 8)
                  + FBBOX_ROT];
        Matrix_RotYXZsuperpack(&mesh_rots, 7);
    } else {
        Matrix_RotYXZsuperpack(&mesh_rots, 6);
    }
    Matrix_RotYXZ(g_Lara.torso_y_rot, g_Lara.torso_x_rot, g_Lara.torso_z_rot);
    Matrix_Push();
    mesh = g_Lara.mesh_ptrs[LM_TORSO];
    Matrix_TranslateRel(mesh[0], mesh[1], mesh[2]);
    spheres[1].x = g_MatrixPtr->_03 >> W2V_SHIFT;
    spheres[1].y = g_MatrixPtr->_13 >> W2V_SHIFT;
    spheres[1].z = g_MatrixPtr->_23 >> W2V_SHIFT;
    spheres[1].r = mesh[3];
    Matrix_Pop();

    Matrix_Push();
    Matrix_TranslateRel(bone[29], bone[30], bone[31]);
    Matrix_RotYXZsuperpack(&mesh_rots, 0);
    mesh = g_Lara.mesh_ptrs[LM_UARM_R];
    Matrix_TranslateRel(mesh[0], mesh[1], mesh[2]);
    spheres[3].y = g_MatrixPtr->_13 >> W2V_SHIFT;
    spheres[3].x = g_MatrixPtr->_03 >> W2V_SHIFT;
    spheres[3].z = g_MatrixPtr->_23 >> W2V_SHIFT;
    spheres[3].r = mesh[3] * 3 / 2;
    Matrix_Pop();

    Matrix_Push();
    Matrix_TranslateRel(bone[41], bone[42], bone[43]);
    Matrix_RotYXZsuperpack(&mesh_rots, 2);
    mesh = g_Lara.mesh_ptrs[LM_UARM_R];
    Matrix_TranslateRel(mesh[0], mesh[1], mesh[2]);
    spheres[4].y = g_MatrixPtr->_13 >> W2V_SHIFT;
    spheres[4].x = g_MatrixPtr->_03 >> W2V_SHIFT;
    spheres[4].z = g_MatrixPtr->_23 >> W2V_SHIFT;
    spheres[4].r = mesh[3] * 3 / 2;
    Matrix_Pop();

    Matrix_TranslateRel(bone[53], bone[54], bone[55]);
    Matrix_RotYXZsuperpack(&mesh_rots, 2);
    Matrix_RotYXZ(g_Lara.head_y_rot, g_Lara.head_x_rot, g_Lara.head_z_rot);
    Matrix_Push();
    mesh = g_Lara.mesh_ptrs[LM_HEAD];
    Matrix_TranslateRel(mesh[0], mesh[1], mesh[2]);
    spheres[2].x = g_MatrixPtr->_03 >> W2V_SHIFT;
    spheres[2].y = g_MatrixPtr->_13 >> W2V_SHIFT;
    spheres[2].z = g_MatrixPtr->_23 >> W2V_SHIFT;
    spheres[2].r = mesh[3];
    Matrix_Pop();

    Matrix_TranslateRel(0, -23, -55);
    const XYZ_32 pos = {
        .x = g_MatrixPtr->_03 >> W2V_SHIFT,
        .y = g_MatrixPtr->_13 >> W2V_SHIFT,
        .z = g_MatrixPtr->_23 >> W2V_SHIFT,
    };
    Matrix_Pop();

    bone = &g_AnimBones[g_Objects[O_LARA_HAIR].bone_idx];

    HAIR_SEGMENT *const fs = &m_HairSegments[0];
    fs->pos.x = pos.x;
    fs->pos.y = pos.y;
    fs->pos.z = pos.z;

    if (m_IsFirstHair) {
        m_IsFirstHair = false;
        for (int32_t i = 1; i <= HAIR_SEGMENTS; i++, bone += 4) {
            const HAIR_SEGMENT *const ps = &m_HairSegments[i - 1];
            HAIR_SEGMENT *const s = &m_HairSegments[i];

            Matrix_PushUnit();
            g_MatrixPtr->_03 = ps->pos.x << W2V_SHIFT;
            g_MatrixPtr->_13 = ps->pos.y << W2V_SHIFT;
            g_MatrixPtr->_23 = ps->pos.z << W2V_SHIFT;
            Matrix_RotYXZ(ps->rot.y, ps->rot.x, 0);
            Matrix_TranslateRel(bone[1], bone[2], bone[3]);

            s->pos.x = g_MatrixPtr->_03 >> W2V_SHIFT;
            s->pos.y = g_MatrixPtr->_13 >> W2V_SHIFT;
            s->pos.z = g_MatrixPtr->_23 >> W2V_SHIFT;

            Matrix_Pop();
        }
        m_HairWind = 0;
        return;
    }

    int16_t room_num = g_LaraItem->room_num;
    int32_t water_height;
    if (in_cutscene) {
        water_height = NO_HEIGHT;
    } else {
        water_height = Room_GetWaterHeight(
            g_LaraItem->pos.x + (frame->bounds.min_x + frame->bounds.max_x) / 2,
            g_LaraItem->pos.y + (frame->bounds.max_y + frame->bounds.min_y) / 2,
            g_LaraItem->pos.z + (frame->bounds.max_z + frame->bounds.min_z) / 2,
            room_num);
    }

    const SECTOR *const sector =
        Room_GetSector(fs->pos.x, fs->pos.y, fs->pos.z, &room_num);
    int32_t height = Room_GetHeight(sector, fs->pos.x, fs->pos.y, fs->pos.z);
    if (height < fs->pos.y) {
        height = g_LaraItem->floor;
    }

    if (g_Rooms[room_num].flags & RF_NOT_INSIDE) {
        const int32_t random = Random_GetDraw() & 7;
        if (random != 0) {
            m_HairWind += random - 4;
            if (m_HairWind < 0) {
                m_HairWind = 0;
            } else if (m_HairWind >= 8) {
                m_HairWind--;
            }
        }
    } else {
        m_HairWind = 0;
    }

    for (int32_t i = 1; i <= HAIR_SEGMENTS; i++, bone += 4) {
        HAIR_SEGMENT *const ps = &m_HairSegments[i - 1];
        HAIR_SEGMENT *const s = &m_HairSegments[i];

        m_HairVelocity[0] = s->pos;

        s->pos.x += m_HairVelocity[i].x * 3 / 4;
        s->pos.y += m_HairVelocity[i].y * 3 / 4;
        s->pos.z += m_HairVelocity[i].z * 3 / 4;

        switch (g_Lara.water_status) {
        case LWS_ABOVE_WATER:
            s->pos.y += 10;
            if (water_height != NO_HEIGHT && s->pos.y > water_height) {
                s->pos.y = water_height;
            } else {
                CLAMPG(s->pos.y, height);
                s->pos.z += m_HairWind;
            }
            break;

        case LWS_UNDERWATER:
        case LWS_SURFACE:
        case LWS_WADE:
            CLAMP(s->pos.y, water_height, height);
            break;

        default:
            break;
        }

        for (int32_t j = 0; j < 5; j++) {
            const SPHERE *const sphere = &spheres[j];
            const int32_t dx = s->pos.x - sphere->x;
            const int32_t dy = s->pos.y - sphere->y;
            const int32_t dz = s->pos.z - sphere->z;
            int32_t dist = SQUARE(dz) + SQUARE(dy) + SQUARE(dx);
            if (dist < SQUARE(sphere->r)) {
                dist = Math_Sqrt(dist);
                CLAMPL(dist, 1);
                s->pos.x = sphere->x + sphere->r * dx / dist;
                s->pos.y = sphere->y + sphere->r * dy / dist;
                s->pos.z = sphere->z + sphere->r * dz / dist;
            }
        }

        const int32_t dx = s->pos.x - ps->pos.x;
        const int32_t dz = s->pos.z - ps->pos.z;
        const int32_t distance = Math_Sqrt(SQUARE(dx) + SQUARE(dz));
        ps->rot.y = Math_Atan(dz, dx);
        ps->rot.x = -Math_Atan(distance, s->pos.y - ps->pos.y);

        Matrix_PushUnit();
        Matrix_TranslateSet(ps->pos.x, ps->pos.y, ps->pos.z);
        Matrix_RotYXZ(ps->rot.y, ps->rot.x, 0);

        if (i == 6) {
            Matrix_TranslateRel(bone[-3], bone[-2], bone[-1]);
        } else {
            Matrix_TranslateRel(bone[1], bone[2], bone[3]);
        }

        s->pos.x = g_MatrixPtr->_03 >> W2V_SHIFT;
        s->pos.y = g_MatrixPtr->_13 >> W2V_SHIFT;
        s->pos.z = g_MatrixPtr->_23 >> W2V_SHIFT;

        m_HairVelocity[i].x = s->pos.x - m_HairVelocity[0].x;
        m_HairVelocity[i].y = s->pos.y - m_HairVelocity[0].y;
        m_HairVelocity[i].z = s->pos.z - m_HairVelocity[0].z;

        Matrix_Pop();
    }
}

void __cdecl Lara_Hair_Draw(void)
{
    int16_t **mesh_ptr = &g_Meshes[g_Objects[O_LARA_HAIR].mesh_idx];
    for (int32_t i = 0; i < HAIR_SEGMENTS; i++) {
        const HAIR_SEGMENT *const s = &m_HairSegments[i];
        Matrix_Push();
        Matrix_TranslateAbs(s->pos.x, s->pos.y, s->pos.z);
        Matrix_RotY(s->rot.y);
        Matrix_RotX(s->rot.x);
        Output_InsertPolygons(*mesh_ptr++, 1);
        Matrix_Pop();
    }
}
