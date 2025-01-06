#include "game/inventory_ring/vars.h"

#include "global/types.h"

#include <stddef.h>
#include <stdint.h>

CLOCK_TIMER g_InvRing_MotionTimer = {};
CAMERA_INFO g_InvRing_OldCamera = {};

INV_RING_SOURCE g_InvRing_Source[RT_NUMBER_OF] = {
    [RT_KEYS] = {
        .current = 0,
        .count = 0,
        .qtys = {
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        },
        .items = {
            &g_InvRing_Item_LeadBar,
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
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
        },
    },
    [RT_MAIN] = {
        .current = 0,
        .count = 1,
        .qtys = {
            1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        },
        .items = {
            &g_InvRing_Item_Compass,
            &g_InvRing_Item_Pistols,
            &g_InvRing_Item_Shotgun,
            &g_InvRing_Item_Magnum,
            &g_InvRing_Item_Uzi,
            &g_InvRing_Item_Grenade,
            &g_InvRing_Item_BigMedi,
            &g_InvRing_Item_Medi,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
        },
    },
    [RT_OPTION] = {
        .current = 0,
        .count = 5,
        .qtys = {
            1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        },
        .items = {
    &g_InvRing_Item_Game,    &g_InvRing_Item_Controls,  &g_InvRing_Item_Sound,
    &g_InvRing_Item_Details, &g_InvRing_Item_LarasHome,
        },
    },
};

INVENTORY_ITEM g_InvRing_Item_Compass = {
    .object_id = O_COMPASS_OPTION,
    .frames_total = 25,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 10,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 4352,
    .x_rot_pt = 0,
    .x_rot_sel = -8192,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 456,
    .z_trans = 0,
    .meshes_sel = 0b00000101,
    .meshes_drawn = 0b00000101,
    .inv_pos = 0,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Medi = {
    .object_id = O_MEDI_OPTION,
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
    .inv_pos = 7,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_BigMedi = {
    .object_id = O_BIGMEDI_OPTION,
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
    .inv_pos = 6,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_LeadBar = {
    .object_id = O_LEADBAR_OPTION,
    .frames_total = 1,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 0,
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
    .inv_pos = 100,
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

INVENTORY_ITEM g_InvRing_Item_Scion = {
    .object_id = O_SCION_OPTION,
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
    .inv_pos = 109,
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
    .inv_pos = 2,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Magnum = {
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

INVENTORY_ITEM g_InvRing_Item_Uzi = {
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

INVENTORY_ITEM g_InvRing_Item_Grenade = {
    .object_id = O_EXPLOSIVE_OPTION,
    .frames_total = 15,
    .current_frame = 0,
    .goal_frame = 0,
    .open_frame = 14,
    .anim_direction = 1,
    .anim_speed = 1,
    .anim_count = 0,
    .x_rot_pt_sel = 5024,
    .x_rot_pt = 0,
    .x_rot_sel = 0,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 368,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 5,
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
    .object_id = O_SG_AMMO_OPTION,
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
    .object_id = O_MAG_AMMO_OPTION,
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

INVENTORY_ITEM g_InvRing_Item_Game = {
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
    .meshes_sel = 0b00010011,
    .meshes_drawn = 0b00010011,
    .inv_pos = 0,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_Details = {
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
    .x_rot_sel = -6720,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 424,
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
    .x_rot_sel = -2336,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 368,
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
    .x_rot_sel = 1536,
    .x_rot_nosel = 0,
    .x_rot = 0,
    .y_rot_sel = 0,
    .y_rot = 0,
    .y_trans_sel = 0,
    .y_trans = 0,
    .z_trans_sel = 352,
    .z_trans = 0,
    .meshes_sel = -1,
    .meshes_drawn = -1,
    .inv_pos = 3,
    .sprite_list = NULL,
};

INVENTORY_ITEM g_InvRing_Item_LarasHome = {
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
