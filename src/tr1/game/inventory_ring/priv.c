#include "game/inventory_ring/priv.h"

#include "game/inventory.h"
#include "game/output.h"
#include "global/vars.h"
#include <libtrx/game/math.h>

static GAME_OBJECT_ID m_RequestedObjectID = NO_OBJECT;

static void M_HandleRequestedObject(RING_INFO *ring);

static void M_HandleRequestedObject(RING_INFO *const ring)
{
    if (m_RequestedObjectID == NO_OBJECT) {
        return;
    }

    for (int32_t i = 0; i < ring->number_of_objects; i++) {
        const GAME_OBJECT_ID item_id = ring->list[i]->object_id;
        if (item_id == m_RequestedObjectID && Inv_RequestItem(item_id) > 0) {
            ring->current_object = i;
            break;
        }
    }

    m_RequestedObjectID = NO_OBJECT;
}

void InvRing_SetRequestedObjectID(const GAME_OBJECT_ID object_id)
{
    m_RequestedObjectID = object_id;
}

void InvRing_InitRing(
    RING_INFO *ring, int16_t type, INVENTORY_ITEM **list, int16_t qty,
    int16_t current, IMOTION_INFO *imo)
{
    ring->type = type;
    ring->radius = 0;
    ring->list = list;
    ring->number_of_objects = qty;
    ring->current_object = current;
    ring->angle_adder = 0x10000 / qty;

    M_HandleRequestedObject(ring);

    if (g_InvMode == INV_TITLE_MODE) {
        ring->camera_pitch = 1024;
    } else {
        ring->camera_pitch = 0;
    }
    ring->rotating = 0;
    ring->rot_count = 0;
    ring->target_object = 0;
    ring->rot_adder = 0;
    ring->rot_adder_l = 0;
    ring->rot_adder_r = 0;

    ring->imo = imo;

    ring->camera.pos.x = 0;
    ring->camera.pos.y = CAMERA_STARTHEIGHT;
    ring->camera.pos.z = 896;
    ring->camera.rot.x = 0;
    ring->camera.rot.y = 0;
    ring->camera.rot.z = 0;

    InvRing_MotionInit(ring, OPEN_FRAMES, RNG_OPENING, RNG_OPEN);
    InvRing_MotionRadius(ring, RING_RADIUS);
    InvRing_MotionCameraPos(ring, CAMERA_HEIGHT);
    InvRing_MotionRotation(
        ring, OPEN_ROTATION,
        0xC000 - (ring->current_object * ring->angle_adder));

    ring->ringpos.pos.x = 0;
    ring->ringpos.pos.y = 0;
    ring->ringpos.pos.z = 0;
    ring->ringpos.rot.x = 0;
    ring->ringpos.rot.y = imo->rotate_target - OPEN_ROTATION;
    ring->ringpos.rot.z = 0;

    ring->light.x = -1536;
    ring->light.y = 256;
    ring->light.z = 1024;
}

void InvRing_ResetItem(INVENTORY_ITEM *const inv_item)
{
    inv_item->drawn_meshes = inv_item->which_meshes;
    inv_item->current_frame = 0;
    inv_item->goal_frame = inv_item->current_frame;
    inv_item->pt_xrot = 0;
    inv_item->x_rot = 0;
    inv_item->y_rot = 0;
    inv_item->ytrans = 0;
    inv_item->ztrans = 0;
    inv_item->action = ACTION_USE;
    if (inv_item->object_id == O_PASSPORT_OPTION) {
        inv_item->object_id = O_PASSPORT_CLOSED;
    }
}

void InvRing_GetView(RING_INFO *ring, XYZ_32 *view_pos, XYZ_16 *view_rot)
{
    PHD_ANGLE angles[2];

    Math_GetVectorAngles(
        -ring->camera.pos.x, CAMERA_YOFFSET - ring->camera.pos.y,
        ring->radius - ring->camera.pos.z, angles);
    view_pos->x = ring->camera.pos.x;
    view_pos->y = ring->camera.pos.y;
    view_pos->z = ring->camera.pos.z;
    view_rot->x = angles[1] + ring->camera_pitch;
    view_rot->y = angles[0];
    view_rot->z = 0;
}

void InvRing_Light(RING_INFO *ring)
{
    PHD_ANGLE angles[2];
    Output_SetLightDivider(0x6000);
    Math_GetVectorAngles(ring->light.x, ring->light.y, ring->light.z, angles);
    Output_RotateLight(angles[1], angles[0]);
}

void InvRing_CalcAdders(RING_INFO *ring, int16_t rotation_duration)
{
    ring->angle_adder = 0x10000 / ring->number_of_objects;
    ring->rot_adder_l = ring->angle_adder / rotation_duration;
    ring->rot_adder_r = -ring->rot_adder_l;
}

void InvRing_DoMotions(RING_INFO *ring)
{
    IMOTION_INFO *imo = ring->imo;

    if (imo->count) {
        ring->radius += imo->radius_rate;
        ring->camera.pos.y += imo->camera_yrate;
        ring->ringpos.rot.y += imo->rotate_rate;
        ring->camera_pitch += imo->camera_pitch_rate;

        INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
        inv_item->pt_xrot += imo->item_ptxrot_rate;
        inv_item->x_rot += imo->item_xrot_rate;
        inv_item->ytrans += imo->item_ytrans_rate;
        inv_item->ztrans += imo->item_ztrans_rate;

        imo->count--;
        if (!imo->count) {
            imo->status = imo->status_target;
            if (imo->radius_rate) {
                imo->radius_rate = 0;
                ring->radius = imo->radius_target;
            }
            if (imo->camera_yrate) {
                imo->camera_yrate = 0;
                ring->camera.pos.y = imo->camera_ytarget;
            }
            if (imo->rotate_rate) {
                imo->rotate_rate = 0;
                ring->ringpos.rot.y = imo->rotate_target;
            }
            if (imo->item_ptxrot_rate) {
                imo->item_ptxrot_rate = 0;
                inv_item->pt_xrot = imo->item_ptxrot_target;
            }
            if (imo->item_xrot_rate) {
                imo->item_xrot_rate = 0;
                inv_item->x_rot = imo->item_xrot_target;
            }
            if (imo->item_ytrans_rate) {
                imo->item_ytrans_rate = 0;
                inv_item->ytrans = imo->item_ytrans_target;
            }
            if (imo->item_ztrans_rate) {
                imo->item_ztrans_rate = 0;
                inv_item->ztrans = imo->item_ztrans_target;
            }
            if (imo->camera_pitch_rate) {
                imo->camera_pitch_rate = 0;
                ring->camera_pitch = imo->camera_pitch_target;
            }
        }
    }

    if (ring->rotating) {
        ring->ringpos.rot.y += ring->rot_adder;
        ring->rot_count--;
        if (!ring->rot_count) {
            ring->current_object = ring->target_object;
            ring->ringpos.rot.y =
                0xC000 - (ring->current_object * ring->angle_adder);
            ring->rotating = 0;
        }
    }
}

void InvRing_RotateLeft(RING_INFO *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object - 1;
    if (ring->target_object < 0) {
        ring->target_object = ring->number_of_objects - 1;
    }
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adder_l;
}

void InvRing_RotateRight(RING_INFO *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object + 1;
    if (ring->target_object >= ring->number_of_objects) {
        ring->target_object = 0;
    }
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adder_r;
}

void InvRing_MotionInit(
    RING_INFO *ring, int16_t frames, int16_t status, int16_t status_target)
{
    ring->imo->status_target = status_target;
    ring->imo->count = frames;
    ring->imo->status = status;
    ring->imo->radius_target = 0;
    ring->imo->radius_rate = 0;
    ring->imo->camera_ytarget = 0;
    ring->imo->camera_yrate = 0;
    ring->imo->camera_pitch_target = 0;
    ring->imo->camera_pitch_rate = 0;
    ring->imo->rotate_target = 0;
    ring->imo->rotate_rate = 0;
    ring->imo->item_ptxrot_target = 0;
    ring->imo->item_ptxrot_rate = 0;
    ring->imo->item_xrot_target = 0;
    ring->imo->item_xrot_rate = 0;
    ring->imo->item_ytrans_target = 0;
    ring->imo->item_ytrans_rate = 0;
    ring->imo->item_ztrans_target = 0;
    ring->imo->item_ztrans_rate = 0;
    ring->imo->misc = 0;
}

void InvRing_MotionSetup(
    RING_INFO *ring, int16_t status, int16_t status_target, int16_t frames)
{
    IMOTION_INFO *imo = ring->imo;
    imo->count = frames;
    imo->status = status;
    imo->status_target = status_target;
    imo->radius_rate = 0;
    imo->camera_yrate = 0;
}

void InvRing_MotionRadius(RING_INFO *ring, int16_t target)
{
    IMOTION_INFO *imo = ring->imo;
    imo->radius_target = target;
    imo->radius_rate = (target - ring->radius) / imo->count;
}

void InvRing_MotionRotation(RING_INFO *ring, int16_t rotation, int16_t target)
{
    IMOTION_INFO *imo = ring->imo;
    imo->rotate_target = target;
    imo->rotate_rate = rotation / imo->count;
}

void InvRing_MotionCameraPos(RING_INFO *ring, int16_t target)
{
    IMOTION_INFO *imo = ring->imo;
    imo->camera_ytarget = target;
    imo->camera_yrate = (target - ring->camera.pos.y) / imo->count;
}

void InvRing_MotionCameraPitch(RING_INFO *ring, int16_t target)
{
    IMOTION_INFO *imo = ring->imo;
    imo->camera_pitch_target = target;
    imo->camera_pitch_rate = target / imo->count;
}

void InvRing_MotionItemSelect(RING_INFO *ring, INVENTORY_ITEM *inv_item)
{
    IMOTION_INFO *imo = ring->imo;
    imo->item_ptxrot_target = inv_item->pt_xrot_sel;
    imo->item_ptxrot_rate = inv_item->pt_xrot_sel / imo->count;
    imo->item_xrot_target = inv_item->x_rot_sel;
    imo->item_xrot_rate = inv_item->x_rot_sel / imo->count;
    imo->item_ytrans_target = inv_item->ytrans_sel;
    imo->item_ytrans_rate = inv_item->ytrans_sel / imo->count;
    imo->item_ztrans_target = inv_item->ztrans_sel;
    imo->item_ztrans_rate = inv_item->ztrans_sel / imo->count;
}

void InvRing_MotionItemDeselect(RING_INFO *ring, INVENTORY_ITEM *inv_item)
{
    IMOTION_INFO *imo = ring->imo;
    imo->item_ptxrot_target = 0;
    imo->item_ptxrot_rate = -inv_item->pt_xrot_sel / imo->count;
    imo->item_xrot_target = 0;
    imo->item_xrot_rate = -inv_item->x_rot_sel / imo->count;
    imo->item_ytrans_target = 0;
    imo->item_ytrans_rate = -inv_item->ytrans_sel / imo->count;
    imo->item_ztrans_target = 0;
    imo->item_ztrans_rate = -inv_item->ztrans_sel / imo->count;
}
