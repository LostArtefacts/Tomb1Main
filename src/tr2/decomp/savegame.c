#include "decomp/savegame.h"

#include "decomp/skidoo.h"
#include "game/inventory.h"
#include "game/items.h"
#include "game/lara/control.h"
#include "game/lara/misc.h"
#include "game/lot.h"
#include "game/objects/creatures/dragon.h"
#include "game/objects/general/lift.h"
#include "game/objects/general/movable_block.h"
#include "game/objects/general/pickup.h"
#include "game/objects/general/puzzle_hole.h"
#include "game/objects/vars.h"
#include "game/requester.h"
#include "game/room.h"
#include "game/shell.h"
#include "global/const.h"
#include "global/vars.h"

#include <libtrx/filesystem.h>

#include <stdio.h>
#include <string.h>

#define MAX_SG_BUFFER_SIZE 6272
#define SAVE_CREATURE (1 << 7)

#define SPECIAL_READ_WRITES                                                    \
    SPECIAL_READ_WRITE(S8, int8_t);                                            \
    SPECIAL_READ_WRITE(S16, int16_t);                                          \
    SPECIAL_READ_WRITE(S32, int32_t);                                          \
    SPECIAL_READ_WRITE(U8, uint8_t);                                           \
    SPECIAL_READ_WRITE(U16, uint16_t);                                         \
    SPECIAL_READ_WRITE(U32, uint32_t);

static int32_t m_BufPos = 0;
static char *m_BufPtr = NULL;
static char *m_BufCopy = NULL;
static uint32_t m_ReqFlags1[MAX_REQUESTER_ITEMS];
static uint32_t m_ReqFlags2[MAX_REQUESTER_ITEMS];

static void M_Read(void *ptr, size_t size);
#undef SPECIAL_READ_WRITE
#define SPECIAL_READ_WRITE(name, type) static type M_Read##name(void);
SPECIAL_READ_WRITES;
static void M_Skip(size_t size);
static void M_ReadItems(void);
static void M_ReadLara(LARA_INFO *lara);
static void M_ReadLaraArm(LARA_ARM *arm);
static void M_ReadAmmoInfo(AMMO_INFO *ammo_info);
static void M_ReadFlares(void);

static void M_Write(const void *ptr, size_t size);
#undef SPECIAL_READ_WRITE
#define SPECIAL_READ_WRITE(name, type) static void M_Write##name(type value);
SPECIAL_READ_WRITES;
static void M_WriteItems(void);
static void M_WriteLara(const LARA_INFO *lara);
static void M_WriteLaraArm(const LARA_ARM *arm);
static void M_WriteAmmoInfo(const AMMO_INFO *ammo_info);
static void M_WriteFlares(void);

static void M_Read(void *ptr, const size_t size)
{
    ReadSG(ptr, size);
}

#undef SPECIAL_READ_WRITE
#define SPECIAL_READ_WRITE(name, type)                                         \
    static type M_Read##name(void)                                             \
    {                                                                          \
        type result;                                                           \
        ReadSG(&result, sizeof(type));                                         \
        return result;                                                         \
    }
SPECIAL_READ_WRITES;

#undef SPECIAL_READ_WRITE
#define SPECIAL_READ_WRITE(name, type)                                         \
    static void M_Write##name(type value)                                      \
    {                                                                          \
        M_Write(&value, sizeof(type));                                         \
    }
SPECIAL_READ_WRITES;

static void M_Skip(const size_t size)
{
    m_BufPos += size;
    m_BufPtr += size;
}

static void M_ReadItems(void)
{
    for (int32_t item_num = 0; item_num < g_LevelItemCount; item_num++) {
        ITEM *const item = Item_Get(item_num);
        OBJECT *const obj = &g_Objects[item->object_id];

        if (Object_IsObjectType(item->object_id, g_MovableBlockObjects)) {
            Room_AlterFloorHeight(item, WALL_L);
        }

        if ((obj->flags & 4) != 0) {
            item->pos.x = M_ReadS32();
            item->pos.y = M_ReadS32();
            item->pos.z = M_ReadS32();
            item->rot.x = M_ReadS16();
            item->rot.y = M_ReadS16();
            item->rot.z = M_ReadS16();
            int16_t room_num = M_ReadS16();
            item->speed = M_ReadS16();
            item->fall_speed = M_ReadS16();

            if (item->room_num != room_num) {
                Item_NewRoom(item_num, room_num);
            }

            if (obj->shadow_size != 0) {
                const SECTOR *const sector = Room_GetSector(
                    item->pos.x, item->pos.y, item->pos.z, &room_num);
                item->floor = Room_GetHeight(
                    sector, item->pos.x, item->pos.y, item->pos.z);
            }
        }

        if (obj->save_anim) {
            item->current_anim_state = M_ReadS16();
            item->goal_anim_state = M_ReadS16();
            item->required_anim_state = M_ReadS16();
            item->anim_num = M_ReadS16();
            item->frame_num = M_ReadS16();
        }

        if (obj->save_hitpoints) {
            item->hit_points = M_ReadS16();
        }

        if (obj->save_flags) {
            item->flags = M_ReadU16();

            if (obj->intelligent) {
                item->carried_item = M_ReadS16();
            }
            item->timer = M_ReadS16();

            if (item->flags & IF_KILLED) {
                Item_Kill(item_num);
                item->status = IS_DEACTIVATED;
            } else {
                if ((item->flags & 1) && !item->active) {
                    Item_AddActive(item_num);
                }

                item->status = (item->flags & 6) >> 1;
                if (item->flags & 8) {
                    item->gravity = 1;
                }
                if (!(item->flags & 0x10)) {
                    item->collidable = 0;
                }
            }

            if (item->flags & SAVE_CREATURE) {
                LOT_EnableBaddieAI(item_num, true);
                CREATURE *const creature = item->data;
                if (creature != NULL) {
                    creature->head_rotation = M_ReadS16();
                    creature->neck_rotation = M_ReadS16();
                    creature->maximum_turn = M_ReadS16();
                    creature->flags = M_ReadS16();
                    creature->mood = M_ReadS32();
                } else {
                    M_Skip(12);
                }
            } else if (obj->intelligent) {
                item->data = NULL;
                if (item->killed && item->hit_points <= 0
                    && !(item->flags & IF_KILLED)) {
                    item->next_active = g_PrevItemActive;
                    g_PrevItemActive = item_num;
                }
            }

            item->flags &= 0xFF00;

            if (Object_IsObjectType(item->object_id, g_PuzzleHoleObjects)
                && (item->status == IS_DEACTIVATED
                    || item->status == IS_ACTIVE)) {
                item->object_id += O_PUZZLE_DONE_1 - O_PUZZLE_HOLE_1;
            }

            if (Object_IsObjectType(item->object_id, g_PickupObjects)
                && item->status == IS_DEACTIVATED) {
                Item_RemoveDrawn(item_num);
            }

            if ((item->object_id == O_WINDOW_1 || item->object_id == O_WINDOW_2)
                && (item->flags & IF_ONE_SHOT)) {
                item->mesh_bits = 0x100;
            }

            if (item->object_id == O_MINE && (item->flags & IF_ONE_SHOT)) {
                item->mesh_bits = 1;
            }
        }

        if (Object_IsObjectType(item->object_id, g_MovableBlockObjects)
            && item->status == IS_INACTIVE) {
            Room_AlterFloorHeight(item, -WALL_L);
        }

        switch (item->object_id) {
        case O_BOAT:
            M_Read(item->data, sizeof(BOAT_INFO));
            break;

        case O_SKIDOO_FAST:
            M_Read(item->data, sizeof(SKIDOO_INFO));
            break;

        case O_LIFT:
            M_Read(item->data, sizeof(LIFT_INFO));
            break;
        }

        if (item->object_id == O_SKIDOO_DRIVER
            && item->status == IS_DEACTIVATED) {
            const int16_t skidoo_num = (int16_t)(intptr_t)item->data;
            ITEM *const skidoo = Item_Get(skidoo_num);
            skidoo->object_id = O_SKIDOO_FAST;
            Skidoo_Initialise(skidoo_num);
        }

        if (item->object_id == O_DRAGON_FRONT
            && item->status == IS_DEACTIVATED) {
            item->pos.y -= 1010;
            Dragon_Bones(item_num);
            item->pos.y += 1010;
        }
    }
}

static void M_ReadLara(LARA_INFO *const lara)
{
    lara->item_num = M_ReadS16();
    lara->gun_status = M_ReadS16();
    lara->gun_type = M_ReadS16();
    lara->request_gun_type = M_ReadS16();
    lara->last_gun_type = M_ReadS16();
    lara->calc_fall_speed = M_ReadS16();
    lara->water_status = M_ReadS16();
    lara->climb_status = M_ReadS16();
    lara->pose_count = M_ReadS16();
    lara->hit_frame = M_ReadS16();
    lara->hit_direction = M_ReadS16();
    lara->air = M_ReadS16();
    lara->dive_count = M_ReadS16();
    lara->death_timer = M_ReadS16();
    lara->current_active = M_ReadS16();
    lara->spaz_effect_count = M_ReadS16();
    lara->flare_age = M_ReadS16();
    lara->skidoo = M_ReadS16();
    lara->weapon_item = M_ReadS16();
    lara->back_gun = M_ReadS16();
    lara->flare_frame = M_ReadS16();

    const uint16_t flags = M_ReadU16();
    // clang-format off
    lara->flare_control_left  = flags >> 0;
    lara->flare_control_right = flags >> 1;
    lara->extra_anim          = flags >> 2;
    lara->look                = flags >> 3;
    lara->burn                = flags >> 4;
    // clang-format on

    lara->water_surface_dist = M_ReadS32();
    lara->last_pos.x = M_ReadS32();
    lara->last_pos.y = M_ReadS32();
    lara->last_pos.z = M_ReadS32();
    M_Skip(4);
    lara->spaz_effect = NULL;
    lara->mesh_effects = M_ReadU32();

    for (int32_t i = 0; i < LM_NUMBER_OF; i++) {
        lara->mesh_ptrs[i] =
            (int16_t *)((intptr_t)g_MeshBase + (intptr_t)M_ReadS32());
    }

    M_Skip(4);
    lara->target = NULL;
    lara->target_angles[0] = M_ReadS16();
    lara->target_angles[1] = M_ReadS16();

    lara->turn_rate = M_ReadS16();
    lara->move_angle = M_ReadS16();
    lara->head_y_rot = M_ReadS16();
    lara->head_x_rot = M_ReadS16();
    lara->head_z_rot = M_ReadS16();
    lara->torso_y_rot = M_ReadS16();
    lara->torso_x_rot = M_ReadS16();
    lara->torso_z_rot = M_ReadS16();

    M_ReadLaraArm(&lara->left_arm);
    M_ReadLaraArm(&lara->right_arm);
    M_ReadAmmoInfo(&lara->pistol_ammo);
    M_ReadAmmoInfo(&lara->magnum_ammo);
    M_ReadAmmoInfo(&lara->uzi_ammo);
    M_ReadAmmoInfo(&lara->shotgun_ammo);
    M_ReadAmmoInfo(&lara->harpoon_ammo);
    M_ReadAmmoInfo(&lara->grenade_ammo);
    M_ReadAmmoInfo(&lara->m16_ammo);
    M_Skip(4);
    lara->creature = NULL;
}

static void M_ReadLaraArm(LARA_ARM *const arm)
{
    arm->frame_base =
        (int16_t *)((intptr_t)g_AnimFrames + (intptr_t)M_ReadS32());
    arm->frame_num = M_ReadS16();
    arm->anim_num = M_ReadS16();
    arm->lock = M_ReadS16();
    arm->rot.y = M_ReadS16();
    arm->rot.x = M_ReadS16();
    arm->rot.z = M_ReadS16();
    arm->flash_gun = M_ReadS16();
}

static void M_ReadAmmoInfo(AMMO_INFO *const ammo_info)
{
    ammo_info->ammo = M_ReadS32();
}

static void M_ReadFlares(void)
{
    const int32_t num_flares = M_ReadS32();
    for (int32_t i = 0; i < num_flares; i++) {
        const int16_t item_num = Item_Create();
        ITEM *const item = Item_Get(item_num);
        item->object_id = O_FLARE_ITEM;
        item->pos.x = M_ReadS32();
        item->pos.y = M_ReadS32();
        item->pos.z = M_ReadS32();
        item->rot.x = M_ReadS16();
        item->rot.y = M_ReadS16();
        item->rot.z = M_ReadS16();
        item->room_num = M_ReadS16();
        item->speed = M_ReadS16();
        item->fall_speed = M_ReadS16();
        Item_Initialise(item_num);
        Item_AddActive(item_num);
        const int32_t flare_age = M_ReadS32();
        item->data = (void *)(intptr_t)flare_age;
    }
}

static void M_Write(const void *ptr, const size_t size)
{
    WriteSG(ptr, size);
}

static void M_WriteItems(void)
{
    for (int32_t i = 0; i < g_LevelItemCount; i++) {
        const ITEM *const item = Item_Get(i);
        const OBJECT *const obj = Object_GetObject(item->object_id);
        if (obj->save_position) {
            M_WriteS32(item->pos.x);
            M_WriteS32(item->pos.y);
            M_WriteS32(item->pos.z);
            M_WriteS16(item->rot.x);
            M_WriteS16(item->rot.y);
            M_WriteS16(item->rot.z);
            M_WriteS16(item->room_num);
            M_WriteS16(item->speed);
            M_WriteS16(item->fall_speed);
        }

        if (obj->save_anim) {
            M_WriteS16(item->current_anim_state);
            M_WriteS16(item->goal_anim_state);
            M_WriteS16(item->required_anim_state);
            M_WriteS16(item->anim_num);
            M_WriteS16(item->frame_num);
        }

        if (obj->save_hitpoints) {
            M_WriteS16(item->hit_points);
        }

        if (obj->save_flags) {
            uint16_t flags = item->flags + item->active + (item->status << 1)
                + (item->gravity << 3) + (item->collidable << 4);
            if (obj->intelligent && item->data != NULL) {
                flags |= SAVE_CREATURE;
            }
            M_WriteU16(flags);
            if (obj->intelligent) {
                M_WriteS16(item->carried_item);
            }

            M_WriteS16(item->timer);
            if (flags & SAVE_CREATURE) {
                const CREATURE *const creature = item->data;
                M_WriteS16(creature->head_rotation);
                M_WriteS16(creature->neck_rotation);
                M_WriteS16(creature->maximum_turn);
                M_WriteS16(creature->flags);
                M_WriteS32(creature->mood);
            }
        }

        switch (item->object_id) {
        case O_BOAT:
            M_Write(item->data, sizeof(BOAT_INFO));
            break;

        case O_SKIDOO_FAST:
            M_Write(item->data, sizeof(SKIDOO_INFO));
            break;

        case O_LIFT:
            M_Write(item->data, sizeof(LIFT_INFO));
            break;
        }
    }
}

static void M_WriteLara(const LARA_INFO *const lara)
{
    M_WriteS16(lara->item_num);
    M_WriteS16(lara->gun_status);
    M_WriteS16(lara->gun_type);
    M_WriteS16(lara->request_gun_type);
    M_WriteS16(lara->last_gun_type);
    M_WriteS16(lara->calc_fall_speed);
    M_WriteS16(lara->water_status);
    M_WriteS16(lara->climb_status);
    M_WriteS16(lara->pose_count);
    M_WriteS16(lara->hit_frame);
    M_WriteS16(lara->hit_direction);
    M_WriteS16(lara->air);
    M_WriteS16(lara->dive_count);
    M_WriteS16(lara->death_timer);
    M_WriteS16(lara->current_active);
    M_WriteS16(lara->spaz_effect_count);
    M_WriteS16(lara->flare_age);
    M_WriteS16(lara->skidoo);
    M_WriteS16(lara->weapon_item);
    M_WriteS16(lara->back_gun);
    M_WriteS16(lara->flare_frame);

    uint16_t flags = 0;
    // clang-format off
    if (lara->flare_control_left)  { flags |= 1 << 0; }
    if (lara->flare_control_right) { flags |= 1 << 1; }
    if (lara->extra_anim)          { flags |= 1 << 2; }
    if (lara->look)                { flags |= 1 << 3; }
    if (lara->burn)                { flags |= 1 << 4; }
    // clang-format on
    M_WriteU16(flags);

    M_WriteS32(lara->water_surface_dist);
    M_WriteS32(lara->last_pos.x);
    M_WriteS32(lara->last_pos.y);
    M_WriteS32(lara->last_pos.z);
    M_Skip(4);
    M_WriteU32(lara->mesh_effects);

    for (int32_t i = 0; i < LM_NUMBER_OF; i++) {
        const int32_t mesh_idx =
            (intptr_t)lara->mesh_ptrs[i] - (intptr_t)g_MeshBase;
        M_WriteS32(mesh_idx);
    }

    M_Skip(4);
    M_WriteS16(lara->target_angles[0]);
    M_WriteS16(lara->target_angles[1]);

    M_WriteS16(lara->turn_rate);
    M_WriteS16(lara->move_angle);
    M_WriteS16(lara->head_y_rot);
    M_WriteS16(lara->head_x_rot);
    M_WriteS16(lara->head_z_rot);
    M_WriteS16(lara->torso_y_rot);
    M_WriteS16(lara->torso_x_rot);
    M_WriteS16(lara->torso_z_rot);

    M_WriteLaraArm(&lara->left_arm);
    M_WriteLaraArm(&lara->right_arm);
    M_WriteAmmoInfo(&lara->pistol_ammo);
    M_WriteAmmoInfo(&lara->magnum_ammo);
    M_WriteAmmoInfo(&lara->uzi_ammo);
    M_WriteAmmoInfo(&lara->shotgun_ammo);
    M_WriteAmmoInfo(&lara->harpoon_ammo);
    M_WriteAmmoInfo(&lara->grenade_ammo);
    M_WriteAmmoInfo(&lara->m16_ammo);
    M_Skip(4);
}

static void M_WriteLaraArm(const LARA_ARM *const arm)
{
    const int32_t frame_base =
        (intptr_t)arm->frame_base - (intptr_t)g_AnimFrames;
    M_WriteS32(frame_base);
    M_WriteS16(arm->frame_num);
    M_WriteS16(arm->anim_num);
    M_WriteS16(arm->lock);
    M_WriteS16(arm->rot.y);
    M_WriteS16(arm->rot.x);
    M_WriteS16(arm->rot.z);
    M_WriteS16(arm->flash_gun);
}

static void M_WriteAmmoInfo(const AMMO_INFO *const ammo_info)
{
    M_WriteS32(ammo_info->ammo);
}

static void M_WriteFlares(void)
{
    int32_t num_flares = 0;
    for (int32_t item_num = 0; item_num < Item_GetTotalCount(); item_num++) {
        const ITEM *const item = Item_Get(item_num);
        if (item->active && item->object_id == O_FLARE_ITEM) {
            num_flares++;
        }
    }

    M_WriteS32(num_flares);
    for (int32_t item_num = 0; item_num < Item_GetTotalCount(); item_num++) {
        const ITEM *const item = Item_Get(item_num);
        if (item->active && item->object_id == O_FLARE_ITEM) {
            M_WriteS32(item->pos.x);
            M_WriteS32(item->pos.y);
            M_WriteS32(item->pos.z);
            M_WriteS16(item->rot.x);
            M_WriteS16(item->rot.y);
            M_WriteS16(item->rot.z);
            M_WriteS16(item->room_num);
            M_WriteS16(item->speed);
            M_WriteS16(item->fall_speed);
            const int32_t flare_age = (intptr_t)item->data;
            M_WriteS32(flare_age);
        }
    }
}

void InitialiseStartInfo(void)
{
    if (g_SaveGame.bonus_flag) {
        return;
    }

    for (int32_t i = 0; i < MAX_LEVELS; i++) {
        ModifyStartInfo(i);
        g_SaveGame.start[i].available = 0;
        g_SaveGame.start[i].statistics.timer = 0;
        g_SaveGame.start[i].statistics.shots = 0;
        g_SaveGame.start[i].statistics.hits = 0;
        g_SaveGame.start[i].statistics.distance = 0;
        g_SaveGame.start[i].statistics.kills = 0;
        g_SaveGame.start[i].statistics.secrets = 0;
        g_SaveGame.start[i].statistics.medipacks = 0;
    }

    g_SaveGame.start[LV_GYM].available = 1;
    g_SaveGame.start[LV_FIRST].available = 1;
    g_SaveGame.bonus_flag = 0;
}

void ModifyStartInfo(const int32_t level_num)
{
    START_INFO *start = &g_SaveGame.start[level_num];
    start->has_pistols = 1;
    start->gun_type = LGT_PISTOLS;
    start->pistol_ammo = 1000;

    if (level_num == LV_GYM) {
        start->available = 1;

        start->has_pistols = 0;
        start->has_shotgun = 0;
        start->has_magnums = 0;
        start->has_uzis = 0;
        start->has_harpoon = 0;
        start->has_m16 = 0;
        start->has_grenade = 0;

        start->pistol_ammo = 0;
        start->shotgun_ammo = 0;
        start->magnum_ammo = 0;
        start->uzi_ammo = 0;
        start->harpoon_ammo = 0;
        start->m16_ammo = 0;
        start->grenade_ammo = 0;

        start->flares = 0;
        start->large_medipacks = 0;
        start->small_medipacks = 0;
        start->gun_type = LGT_UNARMED;
        start->gun_status = LGS_ARMLESS;
    } else if (level_num == LV_FIRST) {
        start->available = 1;

        start->has_pistols = 1;
        start->has_shotgun = 1;
        start->has_magnums = 0;
        start->has_uzis = 0;
        start->has_harpoon = 0;
        start->has_m16 = 0;
        start->has_grenade = 0;

        start->shotgun_ammo = 2 * SHOTGUN_AMMO_CLIP;
        start->magnum_ammo = 0;
        start->uzi_ammo = 0;
        start->harpoon_ammo = 0;
        start->m16_ammo = 0;
        start->grenade_ammo = 0;

        start->flares = 2;
        start->small_medipacks = 1;
        start->large_medipacks = 1;
        start->gun_status = LGS_ARMLESS;
    }

    if (g_SaveGame.bonus_flag && level_num != LV_GYM) {
        start->has_pistols = 1;
        start->has_shotgun = 1;
        start->has_magnums = 1;
        start->has_uzis = 1;
        start->has_grenade = 1;
        start->has_harpoon = 1;
        start->has_m16 = 1;
        start->has_grenade = 1;

        start->shotgun_ammo = 10000;
        start->magnum_ammo = 10000;
        start->uzi_ammo = 10000;
        start->harpoon_ammo = 10000;
        start->m16_ammo = 10000;
        start->grenade_ammo = 10000;

        start->flares = -1;
        start->gun_type = LGT_GRENADE;
    }
}

void CreateStartInfo(const int32_t level_num)
{
    START_INFO *const start = &g_SaveGame.start[level_num];

    start->available = 1;

    if (Inv_RequestItem(O_PISTOL_ITEM)) {
        start->has_pistols = 1;
        start->pistol_ammo = 1000;
    } else {
        start->has_pistols = 0;
        start->pistol_ammo = 1000;
    }

    if (Inv_RequestItem(O_SHOTGUN_ITEM)) {
        start->has_shotgun = 1;
        start->shotgun_ammo = g_Lara.shotgun_ammo.ammo;
    } else {
        start->has_shotgun = 0;
        start->shotgun_ammo =
            Inv_RequestItem(O_SHOTGUN_AMMO_ITEM) * SHOTGUN_AMMO_QTY;
    }

    if (Inv_RequestItem(O_MAGNUM_ITEM)) {
        start->has_magnums = 1;
        start->magnum_ammo = g_Lara.magnum_ammo.ammo;
    } else {
        start->has_magnums = 0;
        start->magnum_ammo =
            Inv_RequestItem(O_MAGNUM_AMMO_ITEM) * MAGNUM_AMMO_QTY;
    }

    if (Inv_RequestItem(O_UZI_ITEM)) {
        start->has_uzis = 1;
        start->uzi_ammo = g_Lara.uzi_ammo.ammo;
    } else {
        start->has_uzis = 0;
        start->uzi_ammo = Inv_RequestItem(O_UZI_AMMO_ITEM) * UZI_AMMO_QTY;
    }

    if (Inv_RequestItem(O_M16_ITEM)) {
        start->has_m16 = 1;
        start->m16_ammo = g_Lara.m16_ammo.ammo;
    } else {
        start->has_m16 = 0;
        start->m16_ammo = Inv_RequestItem(O_M16_AMMO_ITEM) * M16_AMMO_QTY;
    }

    if (Inv_RequestItem(O_HARPOON_ITEM)) {
        start->has_harpoon = 1;
        start->harpoon_ammo = g_Lara.harpoon_ammo.ammo;
    } else {
        start->has_harpoon = 0;
        start->harpoon_ammo =
            Inv_RequestItem(O_HARPOON_AMMO_ITEM) * HARPOON_AMMO_QTY;
    }

    if (Inv_RequestItem(O_GRENADE_ITEM)) {
        start->has_grenade = 1;
        start->grenade_ammo = g_Lara.grenade_ammo.ammo;
    } else {
        start->has_grenade = 0;
        start->grenade_ammo =
            Inv_RequestItem(O_GRENADE_AMMO_ITEM) * GRENADE_AMMO_QTY;
    }

    start->flares = Inv_RequestItem(O_FLARE_ITEM);
    start->small_medipacks = Inv_RequestItem(O_SMALL_MEDIPACK_ITEM);
    start->large_medipacks = Inv_RequestItem(O_LARGE_MEDIPACK_ITEM);

    if (g_Lara.gun_type == LGT_FLARE) {
        start->gun_type = g_Lara.last_gun_type;
    } else {
        start->gun_type = g_Lara.gun_type;
    }
    start->gun_status = LGS_ARMLESS;
}

void CreateSaveGameInfo(void)
{
    g_SaveGame.current_level = g_CurrentLevel;

    CreateStartInfo(g_CurrentLevel);

    // TODO: refactor me!
    g_SaveGame.num_pickup[0] = Inv_RequestItem(O_PICKUP_ITEM_1);
    g_SaveGame.num_pickup[1] = Inv_RequestItem(O_PICKUP_ITEM_2);
    g_SaveGame.num_puzzle[0] = Inv_RequestItem(O_PUZZLE_ITEM_1);
    g_SaveGame.num_puzzle[1] = Inv_RequestItem(O_PUZZLE_ITEM_2);
    g_SaveGame.num_puzzle[2] = Inv_RequestItem(O_PUZZLE_ITEM_3);
    g_SaveGame.num_puzzle[3] = Inv_RequestItem(O_PUZZLE_ITEM_4);
    g_SaveGame.num_key[0] = Inv_RequestItem(O_KEY_ITEM_1);
    g_SaveGame.num_key[1] = Inv_RequestItem(O_KEY_ITEM_2);
    g_SaveGame.num_key[2] = Inv_RequestItem(O_KEY_ITEM_3);
    g_SaveGame.num_key[3] = Inv_RequestItem(O_KEY_ITEM_4);

    ResetSG();
    memset(g_SaveGame.buffer, 0, sizeof(g_SaveGame.buffer));

    M_WriteS32(g_FlipStatus);
    for (int32_t i = 0; i < MAX_FLIP_MAPS; i++) {
        uint8_t tflag = g_FlipMaps[i] >> 8;
        M_WriteU8(tflag);
    }

    M_Write(g_MusicTrackFlags, MAX_CD_TRACKS * sizeof(uint16_t));
    for (int32_t i = 0; i < g_NumCameras; i++) {
        M_WriteS16(g_Camera.fixed[i].flags);
    }

    M_WriteItems();
    M_WriteLara(&g_Lara);

    if (g_Lara.weapon_item != NO_ITEM) {
        const ITEM *const weapon_item = Item_Get(g_Lara.weapon_item);
        M_WriteS16(weapon_item->object_id);
        M_WriteS16(weapon_item->anim_num);
        M_WriteS16(weapon_item->frame_num);
        M_WriteS16(weapon_item->current_anim_state);
        M_WriteS16(weapon_item->goal_anim_state);
    }

    M_WriteS32(g_FlipEffect);
    M_WriteS32(g_FlipTimer);
    M_WriteS32(g_IsMonkAngry);

    M_WriteFlares();
}

void ExtractSaveGameInfo(void)
{
    Lara_InitialiseInventory(g_CurrentLevel);
    Inv_AddItemNTimes(O_PICKUP_ITEM_1, g_SaveGame.num_pickup[0]);
    Inv_AddItemNTimes(O_PICKUP_ITEM_2, g_SaveGame.num_pickup[1]);
    Inv_AddItemNTimes(O_PUZZLE_ITEM_1, g_SaveGame.num_puzzle[0]);
    Inv_AddItemNTimes(O_PUZZLE_ITEM_2, g_SaveGame.num_puzzle[1]);
    Inv_AddItemNTimes(O_PUZZLE_ITEM_3, g_SaveGame.num_puzzle[2]);
    Inv_AddItemNTimes(O_PUZZLE_ITEM_4, g_SaveGame.num_puzzle[3]);
    Inv_AddItemNTimes(O_KEY_ITEM_1, g_SaveGame.num_key[0]);
    Inv_AddItemNTimes(O_KEY_ITEM_2, g_SaveGame.num_key[1]);
    Inv_AddItemNTimes(O_KEY_ITEM_3, g_SaveGame.num_key[2]);
    Inv_AddItemNTimes(O_KEY_ITEM_4, g_SaveGame.num_key[3]);

    ResetSG();

    if (M_ReadS32()) {
        Room_FlipMap();
    }

    for (int32_t i = 0; i < MAX_FLIP_MAPS; i++) {
        g_FlipMaps[i] = M_ReadS8() << 8;
    }

    for (int32_t i = 0; i < MAX_CD_TRACKS; i++) {
        g_MusicTrackFlags[i] = M_ReadU16();
    }

    for (int32_t i = 0; i < g_NumCameras; i++) {
        g_Camera.fixed[i].flags = M_ReadS16();
    }

    M_ReadItems();
    M_ReadLara(&g_Lara);

    if (g_Lara.weapon_item != NO_ITEM) {
        g_Lara.weapon_item = Item_Create();

        ITEM *const weapon_item = Item_Get(g_Lara.weapon_item);
        weapon_item->object_id = M_ReadS16();
        weapon_item->anim_num = M_ReadS16();
        weapon_item->frame_num = M_ReadS16();
        weapon_item->current_anim_state = M_ReadS16();
        weapon_item->goal_anim_state = M_ReadS16();
        weapon_item->status = IS_ACTIVE;
        weapon_item->room_num = NO_ROOM;
    }

    if (g_Lara.burn) {
        g_Lara.burn = 0;
        Lara_CatchFire();
    }

    ReadSG(&g_FlipEffect, sizeof(int32_t));
    ReadSG(&g_FlipTimer, sizeof(int32_t));
    ReadSG(&g_IsMonkAngry, sizeof(int32_t));

    M_ReadFlares();
}

void ResetSG(void)
{
    m_BufPos = 0;
    m_BufPtr = g_SaveGame.buffer;
}

void WriteSG(const void *const pointer, const size_t size)
{
    m_BufPos += size;
    if (m_BufPos >= MAX_SG_BUFFER_SIZE) {
        Shell_ExitSystem("FATAL: Savegame is too big to fit in buffer");
    }

    memcpy(m_BufPtr, pointer, size);
    m_BufPtr += size;
}

void ReadSG(void *const pointer, const size_t size)
{
    m_BufPos += size;
    memcpy(pointer, m_BufPtr, size);
    m_BufPtr += size;
}

void GetSavedGamesList(REQUEST_INFO *const req)
{
    Requester_SetSize(req, 10, -32);
    if (req->selected >= req->visible_count) {
        req->line_offset = req->selected - req->visible_count + 1;
    }
    memcpy(g_RequesterFlags1, m_ReqFlags1, sizeof(m_ReqFlags1));
    memcpy(g_RequesterFlags2, m_ReqFlags2, sizeof(m_ReqFlags2));
}

bool S_FrontEndCheck(void)
{
    Requester_Init(&g_LoadGameRequester);

    g_SavedGames = 0;
    for (int32_t i = 0; i < MAX_REQUESTER_ITEMS; i++) {
        char file_name[80];
        sprintf(file_name, "savegame.%d", i);

        if (!File_Exists(file_name)) {
            Requester_AddItem(
                &g_LoadGameRequester, g_GF_PCStrings[GF_S_PC_EMPTY_SLOT], 0, 0,
                0);
            g_SavedLevels[i] = 0;
        } else {
            MYFILE *const fp = File_Open(file_name, FILE_OPEN_READ);
            char level_title[80];
            File_ReadData(fp, level_title, 75);
            const int32_t save_num = File_ReadS32(fp);
            File_Close(fp);

            char save_num_text[16];
            sprintf(save_num_text, "%d", save_num);

            Requester_AddItem(
                &g_LoadGameRequester, level_title, REQ_ALIGN_LEFT,
                save_num_text, REQ_ALIGN_RIGHT);

            if (save_num > g_SaveCounter) {
                g_SaveCounter = save_num;
                g_LoadGameRequester.selected = i;
            }
            g_SavedLevels[i] = 1;
            g_SavedGames++;
        }
    }

    memcpy(m_ReqFlags1, g_RequesterFlags1, sizeof(m_ReqFlags1));
    memcpy(m_ReqFlags2, g_RequesterFlags2, sizeof(m_ReqFlags2));
    g_SaveCounter++;
    return true;
}

int32_t S_SaveGame(
    const void *const save_data, const size_t save_size, const int32_t slot_num)
{
    char file_name[80];
    sprintf(file_name, "savegame.%d", slot_num);

    MYFILE *const fp = File_Open(file_name, FILE_OPEN_WRITE);
    if (fp == NULL) {
        return false;
    }

    sprintf(file_name, "%s", g_GF_LevelNames[g_SaveGame.current_level]);
    File_WriteData(fp, file_name, 75);
    File_WriteS32(fp, g_SaveCounter);
    File_WriteData(fp, save_data, save_size);
    File_Close(fp);

    char save_num_text[16];
    sprintf(save_num_text, "%d", g_SaveCounter);
    Requester_ChangeItem(
        &g_LoadGameRequester, slot_num, file_name, REQ_ALIGN_LEFT,
        save_num_text, REQ_ALIGN_RIGHT);

    m_ReqFlags1[slot_num] = g_RequesterFlags1[slot_num];
    m_ReqFlags2[slot_num] = g_RequesterFlags2[slot_num];
    g_SavedLevels[slot_num] = 1;
    g_SaveCounter++;
    g_SavedGames++;

    return true;
}

int32_t S_LoadGame(
    void *const save_data, const size_t save_size, const int32_t slot_num)
{
    char file_name[80];
    sprintf(file_name, "savegame.%d", slot_num);

    MYFILE *const fp = File_Open(file_name, FILE_OPEN_READ);
    if (fp == NULL) {
        return false;
    }
    File_Skip(fp, 75);
    File_Skip(fp, 4);
    File_ReadData(fp, save_data, save_size);
    File_Close(fp);
    return true;
}
