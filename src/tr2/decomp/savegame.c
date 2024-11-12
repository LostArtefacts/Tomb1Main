#include "decomp/savegame.h"

#include "game/inventory/backpack.h"
#include "game/objects/general/lift.h"
#include "global/const.h"
#include "global/funcs.h"
#include "global/vars.h"

#define SAVE_CREATURE (1 << 7)

static char *m_BufCopy = NULL;
static void M_Write(const void *ptr, size_t size);
static void M_WriteItems(void);
static void M_WriteLara(const LARA_INFO *lara);
static void M_WriteLaraArm(const LARA_ARM *arm);
static void M_WriteAmmoInfo(const AMMO_INFO *ammo_info);
static void M_WriteFlares(void);

static void M_Write(const void *ptr, size_t size)
{
    WriteSG(ptr, size);
}

static void M_WriteItems(void)
{
    for (int32_t i = 0; i < g_LevelItemCount; i++) {
        const ITEM *const item = Item_Get(i);
        const OBJECT *const obj = Object_GetObject(item->object_id);
        if (obj->save_position) {
            M_Write(&item->pos.x, sizeof(int32_t));
            M_Write(&item->pos.y, sizeof(int32_t));
            M_Write(&item->pos.z, sizeof(int32_t));
            M_Write(&item->rot.x, sizeof(int16_t));
            M_Write(&item->rot.y, sizeof(int16_t));
            M_Write(&item->rot.z, sizeof(int16_t));
            M_Write(&item->room_num, sizeof(int16_t));
            M_Write(&item->speed, sizeof(int16_t));
            M_Write(&item->fall_speed, sizeof(int16_t));
        }

        if (obj->save_anim) {
            M_Write(&item->current_anim_state, sizeof(int16_t));
            M_Write(&item->goal_anim_state, sizeof(int16_t));
            M_Write(&item->required_anim_state, sizeof(int16_t));
            M_Write(&item->anim_num, sizeof(int16_t));
            M_Write(&item->frame_num, sizeof(int16_t));
        }

        if (obj->save_hitpoints) {
            M_Write(&item->hit_points, sizeof(int16_t));
        }

        if (obj->save_flags) {
            uint16_t flags = item->flags + item->active + (item->status << 1)
                + (item->gravity << 3) + (item->collidable << 4);
            if (obj->intelligent && item->data != NULL) {
                flags |= SAVE_CREATURE;
            }
            M_Write(&flags, sizeof(uint16_t));
            if (obj->intelligent) {
                M_Write(&item->carried_item, sizeof(int16_t));
            }

            M_Write(&item->timer, sizeof(int16_t));
            if (flags & SAVE_CREATURE) {
                const CREATURE *const creature = item->data;
                M_Write(&creature->head_rotation, sizeof(int16_t));
                M_Write(&creature->neck_rotation, sizeof(int16_t));
                M_Write(&creature->maximum_turn, sizeof(int16_t));
                M_Write(&creature->flags, sizeof(int16_t));
                M_Write(&creature->mood, sizeof(int32_t));
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
    M_Write(&lara->item_num, sizeof(int16_t));
    M_Write(&lara->gun_status, sizeof(int16_t));
    M_Write(&lara->gun_type, sizeof(int16_t));
    M_Write(&lara->request_gun_type, sizeof(int16_t));
    M_Write(&lara->last_gun_type, sizeof(int16_t));
    M_Write(&lara->calc_fall_speed, sizeof(int16_t));
    M_Write(&lara->water_status, sizeof(int16_t));
    M_Write(&lara->climb_status, sizeof(int16_t));
    M_Write(&lara->pose_count, sizeof(int16_t));
    M_Write(&lara->hit_frame, sizeof(int16_t));
    M_Write(&lara->hit_direction, sizeof(int16_t));
    M_Write(&lara->air, sizeof(int16_t));
    M_Write(&lara->dive_count, sizeof(int16_t));
    M_Write(&lara->death_timer, sizeof(int16_t));
    M_Write(&lara->current_active, sizeof(int16_t));
    M_Write(&lara->spaz_effect_count, sizeof(int16_t));
    M_Write(&lara->flare_age, sizeof(int16_t));
    M_Write(&lara->skidoo, sizeof(int16_t));
    M_Write(&lara->weapon_item, sizeof(int16_t));
    M_Write(&lara->back_gun, sizeof(int16_t));
    M_Write(&lara->flare_frame, sizeof(int16_t));

    uint16_t flags = 0;
    // clang-format off
    if (lara->flare_control_left)  { flags |= 1 << 0; }
    if (lara->flare_control_right) { flags |= 1 << 1; }
    if (lara->extra_anim)          { flags |= 1 << 2; }
    if (lara->look)                { flags |= 1 << 3; }
    if (lara->burn)                { flags |= 1 << 4; }
    // clang-format on
    M_Write(&flags, sizeof(uint16_t));

    M_Write(&lara->water_surface_dist, sizeof(int32_t));
    M_Write(&lara->last_pos.x, sizeof(int32_t));
    M_Write(&lara->last_pos.y, sizeof(int32_t));
    M_Write(&lara->last_pos.z, sizeof(int32_t));
    M_Write(&lara->spaz_effect, sizeof(FX *));
    M_Write(&lara->mesh_effects, sizeof(uint32_t));

    for (int32_t i = 0; i < LM_NUMBER_OF; i++) {
        const int32_t mesh_idx =
            (intptr_t)lara->mesh_ptrs[i] - (intptr_t)g_MeshBase;
        M_Write(&mesh_idx, sizeof(int32_t));
    }

    M_Write(&lara->target, sizeof(ITEM *));
    M_Write(&lara->target_angles[0], sizeof(int16_t));
    M_Write(&lara->target_angles[1], sizeof(int16_t));

    M_Write(&lara->turn_rate, sizeof(int16_t));
    M_Write(&lara->move_angle, sizeof(int16_t));
    M_Write(&lara->head_y_rot, sizeof(int16_t));
    M_Write(&lara->head_x_rot, sizeof(int16_t));
    M_Write(&lara->head_z_rot, sizeof(int16_t));
    M_Write(&lara->torso_y_rot, sizeof(int16_t));
    M_Write(&lara->torso_x_rot, sizeof(int16_t));
    M_Write(&lara->torso_z_rot, sizeof(int16_t));

    M_WriteLaraArm(&lara->left_arm);
    M_WriteLaraArm(&lara->right_arm);
    M_WriteAmmoInfo(&lara->pistol_ammo);
    M_WriteAmmoInfo(&lara->magnum_ammo);
    M_WriteAmmoInfo(&lara->uzi_ammo);
    M_WriteAmmoInfo(&lara->shotgun_ammo);
    M_WriteAmmoInfo(&lara->harpoon_ammo);
    M_WriteAmmoInfo(&lara->grenade_ammo);
    M_WriteAmmoInfo(&lara->m16_ammo);
    M_Write(&lara->creature, sizeof(CREATURE *));
}

static void M_WriteLaraArm(const LARA_ARM *const arm)
{
    const int32_t frame_base =
        (intptr_t)arm->frame_base - (intptr_t)g_AnimFrames;
    M_Write(&frame_base, sizeof(int32_t));
    M_Write(&arm->frame_num, sizeof(int16_t));
    M_Write(&arm->anim_num, sizeof(int16_t));
    M_Write(&arm->lock, sizeof(int16_t));
    M_Write(&arm->rot.y, sizeof(int16_t));
    M_Write(&arm->rot.x, sizeof(int16_t));
    M_Write(&arm->rot.z, sizeof(int16_t));
    M_Write(&arm->flash_gun, sizeof(int16_t));
}

static void M_WriteAmmoInfo(const AMMO_INFO *const ammo_info)
{
    M_Write(&ammo_info->ammo, sizeof(int32_t));
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

    M_Write(&num_flares, sizeof(int32_t));
    for (int32_t item_num = 0; item_num < Item_GetTotalCount(); item_num++) {
        const ITEM *const item = Item_Get(item_num);
        if (item->active && item->object_id == O_FLARE_ITEM) {
            M_Write(&item->pos.x, sizeof(int32_t));
            M_Write(&item->pos.y, sizeof(int32_t));
            M_Write(&item->pos.z, sizeof(int32_t));
            M_Write(&item->rot.x, sizeof(int16_t));
            M_Write(&item->rot.y, sizeof(int16_t));
            M_Write(&item->rot.z, sizeof(int16_t));
            M_Write(&item->room_num, sizeof(int16_t));
            M_Write(&item->speed, sizeof(int16_t));
            M_Write(&item->fall_speed, sizeof(int16_t));
            const int32_t flare_age = (intptr_t)item->data;
            M_Write(&flare_age, sizeof(int32_t));
        }
    }
}

void __cdecl InitialiseStartInfo(void)
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

void __cdecl CreateSaveGameInfo(void)
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

    M_Write(&g_FlipStatus, sizeof(int32_t));
    for (int32_t i = 0; i < MAX_FLIPMAPS; i++) {
        uint8_t tflag = g_FlipMaps[i] >> 8;
        M_Write(&tflag, sizeof(uint8_t));
    }

    M_Write(g_MusicTrackFlags, MAX_CD_TRACKS * sizeof(uint16_t));
    for (int32_t i = 0; i < g_NumCameras; i++) {
        M_Write(&g_Camera.fixed[i].flags, sizeof(int16_t));
    }

    M_WriteItems();
    M_WriteLara(&g_Lara);

    if (g_Lara.weapon_item != NO_ITEM) {
        const ITEM *const weapon_item = Item_Get(g_Lara.weapon_item);
        M_Write(&weapon_item->object_id, sizeof(int16_t));
        M_Write(&weapon_item->anim_num, sizeof(int16_t));
        M_Write(&weapon_item->frame_num, sizeof(int16_t));
        M_Write(&weapon_item->current_anim_state, sizeof(int16_t));
        M_Write(&weapon_item->goal_anim_state, sizeof(int16_t));
    }

    M_Write(&g_FlipEffect, sizeof(int32_t));
    M_Write(&g_FlipTimer, sizeof(int32_t));
    M_Write(&g_IsMonkAngry, sizeof(int32_t));

    M_WriteFlares();
}
