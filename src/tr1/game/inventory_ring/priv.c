#include "game/inventory_ring/priv.h"

#include "game/inventory.h"
#include "game/output.h"
#include "global/vars.h"
#include <libtrx/game/math.h>

static GAME_OBJECT_ID m_RequestedObjectID = NO_OBJECT;

static void M_HandleRequestedObject(INV_RING *ring);

static void M_HandleRequestedObject(INV_RING *const ring)
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
    INV_RING *ring, const RING_TYPE type, INVENTORY_ITEM **list,
    const int16_t qty, const int16_t current)
{
    ring->type = type;
    ring->radius = 0;
    ring->list = list;
    ring->number_of_objects = qty;
    ring->current_object = current;
    ring->angle_adder = 0x10000 / qty;

    ring->is_pass_open = false;
    ring->is_demo_needed = false;
    ring->has_spun_out = false;

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

    ring->camera.pos.x = 0;
    ring->camera.pos.y = CAMERA_STARTHEIGHT;
    ring->camera.pos.z = 896;
    ring->camera.rot.x = 0;
    ring->camera.rot.y = 0;
    ring->camera.rot.z = 0;

    InvRing_MotionInit(ring, RNG_OPENING, RNG_OPEN, OPEN_FRAMES);
    InvRing_MotionRadius(ring, RING_RADIUS);
    InvRing_MotionCameraPos(ring, CAMERA_HEIGHT);
    InvRing_MotionRotation(
        ring, OPEN_ROTATION,
        0xC000 - (ring->current_object * ring->angle_adder));

    ring->ring_pos.pos.x = 0;
    ring->ring_pos.pos.y = 0;
    ring->ring_pos.pos.z = 0;
    ring->ring_pos.rot.x = 0;
    ring->ring_pos.rot.y = ring->motion.rotate_target - OPEN_ROTATION;
    ring->ring_pos.rot.z = 0;

    ring->light.x = -1536;
    ring->light.y = 256;
    ring->light.z = 1024;
}

void InvRing_ResetItem(INVENTORY_ITEM *const inv_item)
{
    inv_item->meshes_drawn = inv_item->meshes_sel;
    inv_item->current_frame = 0;
    inv_item->goal_frame = inv_item->current_frame;
    inv_item->x_rot_pt = 0;
    inv_item->x_rot = 0;
    inv_item->y_rot = 0;
    inv_item->y_trans = 0;
    inv_item->z_trans = 0;
    inv_item->action = ACTION_USE;
    if (inv_item->object_id == O_PASSPORT_OPTION) {
        inv_item->object_id = O_PASSPORT_CLOSED;
    }
}

void InvRing_GetView(INV_RING *ring, XYZ_32 *view_pos, XYZ_16 *view_rot)
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

void InvRing_Light(INV_RING *ring)
{
    PHD_ANGLE angles[2];
    Output_SetLightDivider(0x6000);
    Math_GetVectorAngles(ring->light.x, ring->light.y, ring->light.z, angles);
    Output_RotateLight(angles[1], angles[0]);
}

void InvRing_CalcAdders(INV_RING *ring, int16_t rotation_duration)
{
    ring->angle_adder = 0x10000 / ring->number_of_objects;
    ring->rot_adder_l = ring->angle_adder / rotation_duration;
    ring->rot_adder_r = -ring->rot_adder_l;
}

void InvRing_DoMotions(INV_RING *const ring)
{
    INV_RING_MOTION *const motion = &ring->motion;

    if (motion->count != 0) {
        ring->radius += motion->radius_rate;
        ring->camera.pos.y += motion->camera_y_rate;
        ring->ring_pos.rot.y += motion->rotate_rate;
        ring->camera_pitch += motion->camera_pitch_rate;

        INVENTORY_ITEM *const inv_item = ring->list[ring->current_object];
        inv_item->x_rot_pt += motion->item_pt_x_rot_rate;
        inv_item->x_rot += motion->item_x_rot_rate;
        inv_item->y_trans += motion->item_y_trans_rate;
        inv_item->z_trans += motion->item_z_trans_rate;

        motion->count--;
        if (!motion->count) {
            motion->status = motion->status_target;
            if (motion->radius_rate) {
                motion->radius_rate = 0;
                ring->radius = motion->radius_target;
            }
            if (motion->camera_y_rate) {
                motion->camera_y_rate = 0;
                ring->camera.pos.y = motion->camera_y_target;
            }
            if (motion->rotate_rate) {
                motion->rotate_rate = 0;
                ring->ring_pos.rot.y = motion->rotate_target;
            }
            if (motion->item_pt_x_rot_rate) {
                motion->item_pt_x_rot_rate = 0;
                inv_item->x_rot_pt = motion->item_pt_x_rot_target;
            }
            if (motion->item_x_rot_rate) {
                motion->item_x_rot_rate = 0;
                inv_item->x_rot = motion->item_x_rot_target;
            }
            if (motion->item_y_trans_rate) {
                motion->item_y_trans_rate = 0;
                inv_item->y_trans = motion->item_y_trans_target;
            }
            if (motion->item_z_trans_rate) {
                motion->item_z_trans_rate = 0;
                inv_item->z_trans = motion->item_z_trans_target;
            }
            if (motion->camera_pitch_rate) {
                motion->camera_pitch_rate = 0;
                ring->camera_pitch = motion->camera_pitch_target;
            }
        }
    }

    if (ring->rotating) {
        ring->ring_pos.rot.y += ring->rot_adder;
        ring->rot_count--;
        if (!ring->rot_count) {
            ring->current_object = ring->target_object;
            ring->ring_pos.rot.y =
                0xC000 - (ring->current_object * ring->angle_adder);
            ring->rotating = 0;
        }
    }
}

void InvRing_RotateLeft(INV_RING *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object - 1;
    if (ring->target_object < 0) {
        ring->target_object = ring->number_of_objects - 1;
    }
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adder_l;
}

void InvRing_RotateRight(INV_RING *ring)
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
    INV_RING *ring, const RING_STATUS status, const RING_STATUS status_target,
    const int16_t frames)
{
    INV_RING_MOTION *const motion = &ring->motion;
    motion->count = frames;
    motion->status = status;
    motion->status_target = status_target;

    motion->radius_target = 0;
    motion->radius_rate = 0;
    motion->camera_y_target = 0;
    motion->camera_y_rate = 0;
    motion->camera_pitch_target = 0;
    motion->camera_pitch_rate = 0;
    motion->rotate_target = 0;
    motion->rotate_rate = 0;
    motion->item_pt_x_rot_target = 0;
    motion->item_pt_x_rot_rate = 0;
    motion->item_x_rot_target = 0;
    motion->item_x_rot_rate = 0;
    motion->item_y_trans_target = 0;
    motion->item_y_trans_rate = 0;
    motion->item_z_trans_target = 0;
    motion->item_z_trans_rate = 0;
    motion->misc = 0;
}

void InvRing_MotionSetup(
    INV_RING *ring, const RING_STATUS status, const RING_STATUS status_target,
    const int16_t frames)
{
    INV_RING_MOTION *const motion = &ring->motion;
    motion->count = frames;
    motion->status = status;
    motion->status_target = status_target;
    motion->radius_rate = 0;
    motion->camera_y_rate = 0;
}

void InvRing_MotionRadius(INV_RING *ring, const int16_t target)
{
    INV_RING_MOTION *const motion = &ring->motion;
    motion->radius_target = target;
    motion->radius_rate = (target - ring->radius) / motion->count;
}

void InvRing_MotionRotation(
    INV_RING *ring, const int16_t rotation, const int16_t target)
{
    INV_RING_MOTION *const motion = &ring->motion;
    motion->rotate_target = target;
    motion->rotate_rate = rotation / motion->count;
}

void InvRing_MotionCameraPos(INV_RING *ring, const int16_t target)
{
    INV_RING_MOTION *const motion = &ring->motion;
    motion->camera_y_target = target;
    motion->camera_y_rate = (target - ring->camera.pos.y) / motion->count;
}

void InvRing_MotionCameraPitch(INV_RING *ring, const int16_t target)
{
    INV_RING_MOTION *const motion = &ring->motion;
    motion->camera_pitch_target = target;
    motion->camera_pitch_rate = target / motion->count;
}

void InvRing_MotionItemSelect(INV_RING *ring, INVENTORY_ITEM *inv_item)
{
    INV_RING_MOTION *const motion = &ring->motion;
    motion->item_pt_x_rot_target = inv_item->x_rot_pt_sel;
    motion->item_pt_x_rot_rate = inv_item->x_rot_pt_sel / motion->count;
    motion->item_x_rot_target = inv_item->x_rot_sel;
    motion->item_x_rot_rate = inv_item->x_rot_sel / motion->count;
    motion->item_y_trans_target = inv_item->y_trans_sel;
    motion->item_y_trans_rate = inv_item->y_trans_sel / motion->count;
    motion->item_z_trans_target = inv_item->z_trans_sel;
    motion->item_z_trans_rate = inv_item->z_trans_sel / motion->count;
}

void InvRing_MotionItemDeselect(INV_RING *ring, INVENTORY_ITEM *inv_item)
{
    INV_RING_MOTION *const motion = &ring->motion;
    motion->item_pt_x_rot_target = 0;
    motion->item_pt_x_rot_rate = -inv_item->x_rot_pt_sel / motion->count;
    motion->item_x_rot_target = 0;
    motion->item_x_rot_rate = -inv_item->x_rot_sel / motion->count;
    motion->item_y_trans_target = 0;
    motion->item_y_trans_rate = -inv_item->y_trans_sel / motion->count;
    motion->item_z_trans_target = 0;
    motion->item_z_trans_rate = -inv_item->z_trans_sel / motion->count;
}
