#include "game/game_flow/inventory.h"

#include "game/gun/gun.h"
#include "game/inventory.h"
#include "game/overlay.h"
#include "global/vars.h"

static int8_t m_SecretInvItems[GF_ADD_ITEM_NUMBER_OF] = {};
static int8_t m_Add2InvItems[GF_ADD_ITEM_NUMBER_OF];

static GF_ADD_ITEM M_ModifyInventory_GetGunAdder(LARA_GUN_TYPE gun_type);
static GF_ADD_ITEM M_ModifyInventory_GetAmmoAdder(LARA_GUN_TYPE gun_type);
static GF_ADD_ITEM M_ModifyInventory_GetItemAdder(GAME_OBJECT_ID object_id);
static void M_ModifyInventory_GunOrAmmo(
    START_INFO *start, GF_INV_TYPE type, LARA_GUN_TYPE gun_type);
static void M_ModifyInventory_Item(GF_INV_TYPE type, GAME_OBJECT_ID object_id);

static GF_ADD_ITEM M_ModifyInventory_GetGunAdder(const LARA_GUN_TYPE gun_type)
{
    // clang-format off
    switch (gun_type) {
    case LGT_PISTOLS: return GF_ADD_ITEM_PISTOLS;
    case LGT_MAGNUMS: return GF_ADD_ITEM_MAGNUMS;
    case LGT_UZIS:    return GF_ADD_ITEM_UZIS;
    case LGT_SHOTGUN: return GF_ADD_ITEM_SHOTGUN;
    case LGT_HARPOON: return GF_ADD_ITEM_HARPOON;
    case LGT_M16:     return GF_ADD_ITEM_M16;
    case LGT_GRENADE: return GF_ADD_ITEM_GRENADE;
    default:          return (GF_ADD_ITEM)-1;
    }
    // clang-format on
}

static GF_ADD_ITEM M_ModifyInventory_GetAmmoAdder(const LARA_GUN_TYPE gun_type)
{
    // clang-format off
    switch (gun_type) {
    case LGT_PISTOLS: return GF_ADD_ITEM_PISTOL_AMMO;
    case LGT_MAGNUMS: return GF_ADD_ITEM_MAGNUM_AMMO;
    case LGT_UZIS:    return GF_ADD_ITEM_UZI_AMMO;
    case LGT_SHOTGUN: return GF_ADD_ITEM_SHOTGUN_AMMO;
    case LGT_HARPOON: return GF_ADD_ITEM_HARPOON_AMMO;
    case LGT_M16:     return GF_ADD_ITEM_M16_AMMO;
    case LGT_GRENADE: return GF_ADD_ITEM_GRENADE_AMMO;
    default:          return (GF_ADD_ITEM)-1;
    }
    // clang-format on
}

static GF_ADD_ITEM M_ModifyInventory_GetItemAdder(
    const GAME_OBJECT_ID object_id)
{
    // clang-format off
    switch (object_id) {
    case O_FLARE_ITEM:          return GF_ADD_ITEM_FLARES;
    case O_SMALL_MEDIPACK_ITEM: return GF_ADD_ITEM_SMALL_MEDI;
    case O_LARGE_MEDIPACK_ITEM: return GF_ADD_ITEM_LARGE_MEDI;
    case O_PICKUP_ITEM_1:       return GF_ADD_ITEM_PICKUP_1;
    case O_PICKUP_ITEM_2:       return GF_ADD_ITEM_PICKUP_2;
    case O_PUZZLE_ITEM_1:       return GF_ADD_ITEM_PUZZLE_1;
    case O_PUZZLE_ITEM_2:       return GF_ADD_ITEM_PUZZLE_2;
    case O_PUZZLE_ITEM_3:       return GF_ADD_ITEM_PUZZLE_3;
    case O_PUZZLE_ITEM_4:       return GF_ADD_ITEM_PUZZLE_4;
    case O_KEY_ITEM_1:          return GF_ADD_ITEM_KEY_1;
    case O_KEY_ITEM_2:          return GF_ADD_ITEM_KEY_2;
    case O_KEY_ITEM_3:          return GF_ADD_ITEM_KEY_3;
    case O_KEY_ITEM_4:          return GF_ADD_ITEM_KEY_4;
    default:                    return (GF_ADD_ITEM)-1;
    }
    // clang-format on
}

static void M_ModifyInventory_GunOrAmmo(
    START_INFO *const start, const GF_INV_TYPE type,
    const LARA_GUN_TYPE gun_type)
{
    const GAME_OBJECT_ID gun_item = Gun_GetGunObject(gun_type);
    const GAME_OBJECT_ID ammo_item = Gun_GetAmmoObject(gun_type);
    const int32_t ammo_qty = Gun_GetAmmoQuantity(gun_type);
    AMMO_INFO *const ammo_info = Gun_GetAmmoInfo(gun_type);

    const GF_ADD_ITEM gun_adder = M_ModifyInventory_GetGunAdder(gun_type);
    const GF_ADD_ITEM ammo_adder = M_ModifyInventory_GetAmmoAdder(gun_type);

    if (Inv_RequestItem(gun_item)) {
        if (type == GF_INV_SECRET) {
            ammo_info->ammo += ammo_qty * m_SecretInvItems[ammo_adder];
            for (int32_t i = 0; i < m_SecretInvItems[ammo_adder]; i++) {
                Overlay_AddDisplayPickup(ammo_item);
            }
        } else if (type == GF_INV_REGULAR) {
            ammo_info->ammo += ammo_qty * m_Add2InvItems[ammo_adder];
        }
    } else if (
        (type == GF_INV_REGULAR && m_Add2InvItems[gun_adder])
        || (type == GF_INV_SECRET && m_SecretInvItems[gun_adder])) {

        // clang-format off
        // TODO: consider moving this to Inv_AddItem
        switch (gun_type) {
        case LGT_PISTOLS: start->has_pistols = 1; break;
        case LGT_MAGNUMS: start->has_magnums = 1; break;
        case LGT_UZIS:    start->has_uzis = 1;    break;
        case LGT_SHOTGUN: start->has_shotgun = 1; break;
        case LGT_HARPOON: start->has_harpoon = 1; break;
        case LGT_M16:     start->has_m16 = 1;     break;
        case LGT_GRENADE: start->has_grenade = 1; break;
        default: break;
        }
        // clang-format on

        Inv_AddItem(gun_item);

        if (type == GF_INV_SECRET) {
            ammo_info->ammo += ammo_qty * m_SecretInvItems[ammo_adder];
            Overlay_AddDisplayPickup(gun_item);
            for (int32_t i = 0; i < m_SecretInvItems[ammo_adder]; i++) {
                Overlay_AddDisplayPickup(ammo_item);
            }
        } else if (type == GF_INV_REGULAR) {
            ammo_info->ammo += ammo_qty * m_Add2InvItems[ammo_adder];
        }
    } else if (type == GF_INV_SECRET) {
        for (int32_t i = 0; i < m_SecretInvItems[ammo_adder]; i++) {
            Inv_AddItem(ammo_item);
            Overlay_AddDisplayPickup(ammo_item);
        }
    } else if (type == GF_INV_REGULAR) {
        for (int32_t i = 0; i < m_Add2InvItems[ammo_adder]; i++) {
            Inv_AddItem(ammo_item);
        }
    }
}

static void M_ModifyInventory_Item(
    const GF_INV_TYPE type, const GAME_OBJECT_ID object_id)
{
    const GF_ADD_ITEM item_adder = M_ModifyInventory_GetItemAdder(object_id);
    int32_t qty = 0;
    if (type == GF_INV_SECRET) {
        qty = m_SecretInvItems[item_adder];
    } else if (type == GF_INV_REGULAR) {
        qty = m_Add2InvItems[item_adder];
    }

    for (int32_t i = 0; i < qty; i++) {
        Inv_AddItem(object_id);
        if (type == GF_INV_SECRET) {
            Overlay_AddDisplayPickup(object_id);
        }
    }
}

void GF_InventoryModifier_Reset(void)
{
    for (int32_t i = 0; i < GF_ADD_ITEM_NUMBER_OF; i++) {
        m_SecretInvItems[i] = 0;
        m_Add2InvItems[i] = 0;
    }
}

void GF_InventoryModifier_Add(
    const GF_ADD_ITEM item_type, const GF_INV_TYPE type)
{
    if (type == GF_INV_SECRET) {
        m_SecretInvItems[item_type]++;
    } else if (type == GF_INV_REGULAR) {
        m_Add2InvItems[item_type]++;
    }
}

void GF_InventoryModifier_Apply(const int32_t level, const GF_INV_TYPE type)
{
    START_INFO *const start = &g_SaveGame.start[level];

    if (!start->has_pistols && m_Add2InvItems[GF_ADD_ITEM_PISTOLS]) {
        start->has_pistols = 1;
        Inv_AddItem(O_PISTOL_ITEM);
    }

    M_ModifyInventory_GunOrAmmo(start, type, LGT_MAGNUMS);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_UZIS);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_SHOTGUN);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_HARPOON);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_M16);
    M_ModifyInventory_GunOrAmmo(start, type, LGT_GRENADE);

    M_ModifyInventory_Item(type, O_FLARE_ITEM);
    M_ModifyInventory_Item(type, O_SMALL_MEDIPACK_ITEM);
    M_ModifyInventory_Item(type, O_LARGE_MEDIPACK_ITEM);
    M_ModifyInventory_Item(type, O_PICKUP_ITEM_1);
    M_ModifyInventory_Item(type, O_PICKUP_ITEM_2);
    M_ModifyInventory_Item(type, O_PUZZLE_ITEM_1);
    M_ModifyInventory_Item(type, O_PUZZLE_ITEM_2);
    M_ModifyInventory_Item(type, O_PUZZLE_ITEM_3);
    M_ModifyInventory_Item(type, O_PUZZLE_ITEM_4);
    M_ModifyInventory_Item(type, O_KEY_ITEM_1);
    M_ModifyInventory_Item(type, O_KEY_ITEM_2);
    M_ModifyInventory_Item(type, O_KEY_ITEM_3);
    M_ModifyInventory_Item(type, O_KEY_ITEM_4);

    for (int32_t i = 0; i < GF_ADD_ITEM_NUMBER_OF; i++) {
        if (type == GF_INV_SECRET) {
            m_SecretInvItems[i] = 0;
        } else if (type == GF_INV_REGULAR) {
            m_Add2InvItems[i] = 0;
        }
    }
}
