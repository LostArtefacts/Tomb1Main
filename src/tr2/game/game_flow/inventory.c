#include "game/game_flow/inventory.h"

#include "game/gun/gun.h"
#include "game/inventory.h"
#include "game/overlay.h"
#include "global/vars.h"

static int8_t m_SecretInvItems[O_NUMBER_OF] = {};
static int8_t m_Add2InvItems[O_NUMBER_OF] = {};

static void M_ModifyInventory_GunOrAmmo(
    START_INFO *start, GF_INV_TYPE type, LARA_GUN_TYPE gun_type);
static void M_ModifyInventory_Item(GF_INV_TYPE type, GAME_OBJECT_ID object_id);

static void M_ModifyInventory_GunOrAmmo(
    START_INFO *const start, const GF_INV_TYPE type,
    const LARA_GUN_TYPE gun_type)
{
    const GAME_OBJECT_ID gun_item = Gun_GetGunObject(gun_type);
    const GAME_OBJECT_ID ammo_item = Gun_GetAmmoObject(gun_type);
    const int32_t ammo_qty = Gun_GetAmmoQuantity(gun_type);
    AMMO_INFO *const ammo_info = Gun_GetAmmoInfo(gun_type);

    if (Inv_RequestItem(gun_item)) {
        if (type == GF_INV_SECRET) {
            ammo_info->ammo += ammo_qty * m_SecretInvItems[ammo_item];
            for (int32_t i = 0; i < m_SecretInvItems[ammo_item]; i++) {
                Overlay_AddDisplayPickup(ammo_item);
            }
        } else if (type == GF_INV_REGULAR) {
            ammo_info->ammo += ammo_qty * m_Add2InvItems[ammo_item];
        }
    } else if (
        (type == GF_INV_REGULAR && m_Add2InvItems[gun_item])
        || (type == GF_INV_SECRET && m_SecretInvItems[gun_item])) {

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
            ammo_info->ammo += ammo_qty * m_SecretInvItems[ammo_item];
            Overlay_AddDisplayPickup(gun_item);
            for (int32_t i = 0; i < m_SecretInvItems[ammo_item]; i++) {
                Overlay_AddDisplayPickup(ammo_item);
            }
        } else if (type == GF_INV_REGULAR) {
            ammo_info->ammo += ammo_qty * m_Add2InvItems[ammo_item];
        }
    } else if (type == GF_INV_SECRET) {
        for (int32_t i = 0; i < m_SecretInvItems[ammo_item]; i++) {
            Inv_AddItem(ammo_item);
            Overlay_AddDisplayPickup(ammo_item);
        }
    } else if (type == GF_INV_REGULAR) {
        for (int32_t i = 0; i < m_Add2InvItems[ammo_item]; i++) {
            Inv_AddItem(ammo_item);
        }
    }
}

static void M_ModifyInventory_Item(
    const GF_INV_TYPE type, const GAME_OBJECT_ID object_id)
{
    int32_t qty = 0;
    if (type == GF_INV_SECRET) {
        qty = m_SecretInvItems[object_id];
    } else if (type == GF_INV_REGULAR) {
        qty = m_Add2InvItems[object_id];
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
    for (int32_t i = 0; i < O_NUMBER_OF; i++) {
        m_SecretInvItems[i] = 0;
        m_Add2InvItems[i] = 0;
    }
}

void GF_InventoryModifier_Add(
    const GAME_OBJECT_ID object_id, const GF_INV_TYPE type)
{
    if (object_id < 0 || object_id >= O_NUMBER_OF) {
        return;
    }
    if (type == GF_INV_SECRET) {
        m_SecretInvItems[object_id]++;
    } else if (type == GF_INV_REGULAR) {
        m_Add2InvItems[object_id]++;
    }
}

void GF_InventoryModifier_Apply(const int32_t level, const GF_INV_TYPE type)
{
    START_INFO *const start = &g_SaveGame.start[level];

    if (!start->has_pistols && m_Add2InvItems[O_PISTOL_ITEM]) {
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

    for (int32_t i = 0; i < O_NUMBER_OF; i++) {
        if (type == GF_INV_SECRET) {
            m_SecretInvItems[i] = 0;
        } else if (type == GF_INV_REGULAR) {
            m_Add2InvItems[i] = 0;
        }
    }
}
