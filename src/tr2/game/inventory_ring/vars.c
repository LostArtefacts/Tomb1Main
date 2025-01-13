#include "game/inventory_ring/vars.h"

#include "global/vars.h"

INVENTORY_MODE g_Inv_Mode = INV_TITLE_MODE;
int16_t g_Inv_Chosen = -1;
int32_t g_Inv_ExtraData[8];
bool g_Inv_IsOptionsDelay;
int32_t g_Inv_OptionsDelayCounter;
int32_t g_Inv_NFrames = 2;

INV_RING_SOURCE g_InvRing_Source[RT_NUMBER_OF] = {
    [RT_KEYS] = {
        .count = 0,
        .current = 0,
        .qtys = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
        },
        .items = {
            &g_InvRing_Item_Puzzle1,
            &g_InvRing_Item_Puzzle2,
            &g_InvRing_Item_Puzzle3,
            &g_InvRing_Item_Puzzle4,
            &g_InvRing_Item_Key1,
            &g_InvRing_Item_Key2,
            &g_InvRing_Item_Key3,
            &g_InvRing_Item_Key4,
            &g_InvRing_Item_Pickup1,
            &g_InvRing_Item_Pickup2,
        },
    },
    [RT_MAIN] = {
        .count = 8,
        .current = 0,
        .qtys = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
        },
        .items = {
            &g_InvRing_Item_Stopwatch,
            &g_InvRing_Item_Flare,
            &g_InvRing_Item_Pistols,
            &g_InvRing_Item_Shotgun,
            &g_InvRing_Item_Magnums,
            &g_InvRing_Item_Uzis,
            &g_InvRing_Item_M16,
            &g_InvRing_Item_Grenade,
            &g_InvRing_Item_Harpoon,
            &g_InvRing_Item_LargeMedi,
            &g_InvRing_Item_SmallMedi,
        },
    },
    [RT_OPTION] = {
        .count = 4,
        .current = 0,
        .qtys = {},
        .items = {
            &g_InvRing_Item_Passport,
            &g_InvRing_Item_Controls,
            &g_InvRing_Item_Sound,
            &g_InvRing_Item_Photo,
        },
    },
};

INVENTORY_ITEM g_InvRing_Item_Stopwatch = {
    .object_id = O_COMPASS_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4352,
    .x_rot_pt = 0,
    .x_rot_sel = -1536,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = -170,
    .y_trans = 0,
    .z_trans_sel = 320,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 0,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Pistols = {
    .object_id = O_PISTOL_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 2848,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -32768,
    .y_rot = 0,
    .y_trans_sel = 38,
    .y_trans = 0,
    .z_trans_sel = 352,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 1,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Flare = {
    .object_id = O_FLARES_OPTION,
    .frames_total = 31,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 30,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 0,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -8192,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 8,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Shotgun = {
    .object_id = O_SHOTGUN_OPTION,
    .frames_total = 13,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 12,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 5120,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 30720,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 228,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 2,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Magnums = {
    .object_id = O_MAGNUM_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 3360,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -32768,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 362,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 3,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Uzis = {
    .object_id = O_UZI_OPTION,
    .frames_total = 13,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 12,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = 2336,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -32768,
    .y_rot = 0,
    .y_trans_sel = 56,
    .y_trans = 0,
    .z_trans_sel = 322,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 4,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Harpoon = {
    .object_id = O_HARPOON_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -736,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -19456,
    .y_rot = 0,
    .y_trans_sel = 58,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 7,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_M16 = {
    .object_id = O_M16_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -224,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -18432,
    .y_rot = 0,
    .y_trans_sel = 84,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 5,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Grenade = {
    .object_id = O_GRENADE_OPTION,
    .frames_total = 12,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 11,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -224,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 14336,
    .y_rot = 0,
    .y_trans_sel = 56,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 6,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_PistolAmmo = {
    .object_id = O_PISTOL_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -3808,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 1,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_ShotgunAmmo = {
    .object_id = O_SHOTGUN_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -3808,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 2,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_MagnumAmmo = {
    .object_id = O_MAGNUM_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -3808,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 3,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_UziAmmo = {
    .object_id = O_UZI_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -3808,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 4,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_HarpoonAmmo = {
    .object_id = O_HARPOON_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -3808,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 7,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_M16Ammo = {
    .object_id = O_M16_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -3808,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 5,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_GrenadeAmmo = {
    .object_id = O_GRENADE_AMMO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3200,
    .x_rot_pt = 0,
    .x_rot_sel = -3808,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 296,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 6,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_SmallMedi = {
    .object_id = O_SMALL_MEDIPACK_OPTION,
    .frames_total = 26,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 25,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4032,
    .x_rot_pt = 0,
    .x_rot_sel = -7296,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -4096,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 216,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 11,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_LargeMedi = {
    .object_id = O_LARGE_MEDIPACK_OPTION,
    .frames_total = 20,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 19,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 3616,
    .x_rot_pt = 0,
    .x_rot_sel = -8160,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -4096,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 352,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 10,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Pickup1 = {
    .object_id = O_PICKUP_OPTION_1,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 111,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Pickup2 = {
    .object_id = O_PICKUP_OPTION_2,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 110,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Puzzle1 = {
    .object_id = O_PUZZLE_OPTION_1,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 108,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Puzzle2 = {
    .object_id = O_PUZZLE_OPTION_2,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 107,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Puzzle3 = {
    .object_id = O_PUZZLE_OPTION_3,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 106,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Puzzle4 = {
    .object_id = O_PUZZLE_OPTION_4,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 105,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Key1 = {
    .object_id = O_KEY_OPTION_1,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 101,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Key2 = {
    .object_id = O_KEY_OPTION_2,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 102,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Key3 = {
    .object_id = O_KEY_OPTION_3,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 103,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Key4 = {
    .object_id = O_KEY_OPTION_4,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 7200,
    .x_rot_pt = 0,
    .x_rot_sel = -4352,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 256,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 104,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Passport = {
    .object_id = O_PASSPORT_CLOSED,
    .frames_total = 30,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 14,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4640,
    .x_rot_pt = 0,
    .x_rot_sel = -4320,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 384,
    .z_trans = 0,
    .meshes_sel = 19,
    .meshes_drawn = 19,
    .inv_pos = 0,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Graphics = {
    .object_id = O_DETAIL_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4224,
    .x_rot_pt = 0,
    .x_rot_sel = -7232,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 16,
    .y_trans = 0,
    .z_trans_sel = 444,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 1,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Sound = {
    .object_id = O_SOUND_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4832,
    .x_rot_pt = 0,
    .x_rot_sel = -5408,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = -3072,
    .y_rot = 0,
    .y_trans_sel = -2,
    .y_trans = 0,
    .z_trans_sel = 350,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 2,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Controls = {
    .object_id = O_CONTROL_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 5504,
    .x_rot_pt = 0,
    .x_rot_sel = -2560,
    .x_rot_nosel = 5632,
    .x_rot = 5632,
    .y_rot_sel = 13312,
    .y_rot = 0,
    .y_trans_sel = 46,
    .y_trans = 0,
    .z_trans_sel = 508,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 3,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Photo = {
    .object_id = O_PHOTO_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4640,
    .x_rot_pt = 0,
    .x_rot_sel = -4320,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 384,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 5,
    .sprite_list = NULL,
};
