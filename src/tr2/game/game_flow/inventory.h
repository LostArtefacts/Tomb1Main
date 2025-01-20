#pragma once

#include <stdint.h>

typedef enum {
    GF_ADD_ITEM_PISTOLS = 0,
    GF_ADD_ITEM_SHOTGUN = 1,
    GF_ADD_ITEM_MAGNUMS = 2,
    GF_ADD_ITEM_UZIS = 3,
    GF_ADD_ITEM_HARPOON = 4,
    GF_ADD_ITEM_M16 = 5,
    GF_ADD_ITEM_GRENADE = 6,
    GF_ADD_ITEM_PISTOL_AMMO = 7,
    GF_ADD_ITEM_SHOTGUN_AMMO = 8,
    GF_ADD_ITEM_MAGNUM_AMMO = 9,
    GF_ADD_ITEM_UZI_AMMO = 10,
    GF_ADD_ITEM_HARPOON_AMMO = 11,
    GF_ADD_ITEM_M16_AMMO = 12,
    GF_ADD_ITEM_GRENADE_AMMO = 13,
    GF_ADD_ITEM_FLARES = 14,
    GF_ADD_ITEM_SMALL_MEDI = 15,
    GF_ADD_ITEM_LARGE_MEDI = 16,
    GF_ADD_ITEM_PICKUP_1 = 17,
    GF_ADD_ITEM_PICKUP_2 = 18,
    GF_ADD_ITEM_PUZZLE_1 = 19,
    GF_ADD_ITEM_PUZZLE_2 = 20,
    GF_ADD_ITEM_PUZZLE_3 = 21,
    GF_ADD_ITEM_PUZZLE_4 = 22,
    GF_ADD_ITEM_KEY_1 = 23,
    GF_ADD_ITEM_KEY_2 = 24,
    GF_ADD_ITEM_KEY_3 = 25,
    GF_ADD_ITEM_KEY_4 = 26,
    GF_ADD_ITEM_NUMBER_OF = 27,
} GF_ADD_ITEM;

typedef enum {
    GF_INV_REGULAR,
    GF_INV_SECRET,
} GF_INV_TYPE;

void GF_InventoryModifier_Reset(void);
void GF_InventoryModifier_Add(GF_ADD_ITEM item_type, GF_INV_TYPE type);
void GF_InventoryModifier_Apply(int32_t level, GF_INV_TYPE type);
