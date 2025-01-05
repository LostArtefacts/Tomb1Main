#include "game/inventory_ring/control.h"

#include "game/game_string.h"
#include "game/inventory.h"
#include "game/inventory_ring/priv.h"
#include "game/inventory_ring/vars.h"
#include "game/option/option_examine.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/text.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/math.h>
#include <libtrx/game/objects/names.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

TEXTSTRING *g_InvItemText[IT_NUMBER_OF] = { NULL };
TEXTSTRING *g_InvRingText = NULL;

static TEXTSTRING *m_InvDownArrow1 = NULL;
static TEXTSTRING *m_InvDownArrow2 = NULL;
static TEXTSTRING *m_InvUpArrow1 = NULL;
static TEXTSTRING *m_InvUpArrow2 = NULL;
static TEXTSTRING *m_ExamineItemText = NULL;
static TEXTSTRING *m_UseItemText = NULL;

static TEXTSTRING *M_InitExamineText(
    int32_t x_pos, const char *role_str, const char *input_str);

static TEXTSTRING *M_InitExamineText(
    const int32_t x_pos, const char *const role_str,
    const char *const input_str)
{
    char role[100];
    sprintf(role, role_str, input_str);

    TEXTSTRING *const text = Text_Create(x_pos, -100, role);
    Text_AlignBottom(text, true);
    Text_CentreH(text, true);
    Text_Hide(text, true);

    return text;
}

bool InvRing_CanExamine(void)
{
    return g_Config.gameplay.enable_item_examining && m_ExamineItemText != NULL
        && !m_ExamineItemText->flags.hide;
}

void InvRing_InitExamineOverlay(void)
{
    if ((g_InvMode != INV_GAME_MODE && g_InvMode != INV_KEYS_MODE)
        || !g_Config.gameplay.enable_item_examining
        || m_ExamineItemText != NULL) {
        return;
    }

    m_ExamineItemText =
        M_InitExamineText(-100, GS(ITEM_EXAMINE_ROLE), GS(KEYMAP_LOOK));
    m_UseItemText =
        M_InitExamineText(100, GS(ITEM_USE_ROLE), GS(KEYMAP_ACTION));
}

void InvRing_RemoveExamineOverlay(void)
{
    if (m_ExamineItemText == NULL) {
        return;
    }

    Text_Remove(m_ExamineItemText);
    Text_Remove(m_UseItemText);
    m_ExamineItemText = NULL;
    m_UseItemText = NULL;
}

void InvRing_InitHeader(RING_INFO *ring)
{
    if (g_InvMode == INV_TITLE_MODE) {
        return;
    }

    if (!g_InvRingText) {
        switch (ring->type) {
        case RT_MAIN:
            g_InvRingText = Text_Create(0, 26, GS(HEADING_INVENTORY));
            break;

        case RT_OPTION:
            if (g_InvMode == INV_DEATH_MODE) {
                g_InvRingText = Text_Create(0, 26, GS(HEADING_GAME_OVER));
            } else {
                g_InvRingText = Text_Create(0, 26, GS(HEADING_OPTION));
            }
            break;

        case RT_KEYS:
            g_InvRingText = Text_Create(0, 26, GS(HEADING_ITEMS));
            break;
        }

        Text_CentreH(g_InvRingText, 1);
    }

    if (g_InvMode != INV_GAME_MODE) {
        return;
    }

    if (!m_InvUpArrow1) {
        if (ring->type == RT_OPTION
            || (ring->type == RT_MAIN && g_InvKeysObjects)) {
            m_InvUpArrow1 = Text_Create(20, 28, "\\{arrow up}");
            m_InvUpArrow2 = Text_Create(-20, 28, "\\{arrow up}");
            Text_AlignRight(m_InvUpArrow2, 1);
        }
    }

    if (!m_InvDownArrow1) {
        if (ring->type == RT_MAIN || ring->type == RT_KEYS) {
            m_InvDownArrow1 = Text_Create(20, -15, "\\{arrow down}");
            m_InvDownArrow2 = Text_Create(-20, -15, "\\{arrow down}");
            Text_AlignBottom(m_InvDownArrow1, 1);
            Text_AlignBottom(m_InvDownArrow2, 1);
            Text_AlignRight(m_InvDownArrow2, 1);
        }
    }
}

void InvRing_RemoveHeader(void)
{
    if (!g_InvRingText) {
        return;
    }

    Text_Remove(g_InvRingText);
    g_InvRingText = NULL;

    if (m_InvUpArrow1) {
        Text_Remove(m_InvUpArrow1);
        Text_Remove(m_InvUpArrow2);
        m_InvUpArrow1 = NULL;
        m_InvUpArrow2 = NULL;
    }
    if (m_InvDownArrow1) {
        Text_Remove(m_InvDownArrow1);
        Text_Remove(m_InvDownArrow2);
        m_InvDownArrow1 = NULL;
        m_InvDownArrow2 = NULL;
    }
}

void InvRing_RemoveAllText(void)
{
    InvRing_RemoveHeader();
    InvRing_RemoveExamineOverlay();
    for (int i = 0; i < IT_NUMBER_OF; i++) {
        if (g_InvItemText[i]) {
            Text_Remove(g_InvItemText[i]);
            g_InvItemText[i] = NULL;
        }
    }
}

void InvRing_Active(INVENTORY_ITEM *inv_item)
{
    if (g_InvItemText[IT_NAME] == NULL
        && inv_item->object_id != O_PASSPORT_OPTION) {
        g_InvItemText[IT_NAME] =
            Text_Create(0, -16, Object_GetName(inv_item->object_id));
        Text_AlignBottom(g_InvItemText[IT_NAME], 1);
        Text_CentreH(g_InvItemText[IT_NAME], 1);
    }

    char temp_text[128];
    int32_t qty = Inv_RequestItem(inv_item->object_id);

    bool show_examine_option = false;

    switch (inv_item->object_id) {
    case O_SHOTGUN_OPTION:
        if (!g_InvItemText[IT_QTY] && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
            sprintf(
                temp_text, "%5d A", g_Lara.shotgun.ammo / SHOTGUN_AMMO_CLIP);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }
        break;

    case O_MAGNUM_OPTION:
        if (!g_InvItemText[IT_QTY] && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
            sprintf(temp_text, "%5d B", g_Lara.magnums.ammo);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }
        break;

    case O_UZI_OPTION:
        if (!g_InvItemText[IT_QTY] && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
            sprintf(temp_text, "%5d C", g_Lara.uzis.ammo);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }
        break;

    case O_SG_AMMO_OPTION:
        if (!g_InvItemText[IT_QTY]) {
            sprintf(temp_text, "%d", qty * NUM_SG_SHELLS);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }
        break;

    case O_MAG_AMMO_OPTION:
        if (!g_InvItemText[IT_QTY]) {
            sprintf(temp_text, "%d", Inv_RequestItem(O_MAG_AMMO_OPTION) * 2);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }
        break;

    case O_UZI_AMMO_OPTION:
        if (!g_InvItemText[IT_QTY]) {
            sprintf(temp_text, "%d", Inv_RequestItem(O_UZI_AMMO_OPTION) * 2);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }
        break;

    case O_MEDI_OPTION:
        Overlay_BarSetHealthTimer(40);
        if (!g_InvItemText[IT_QTY] && qty > 1) {
            sprintf(temp_text, "%d", qty);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }
        break;

    case O_BIGMEDI_OPTION:
        Overlay_BarSetHealthTimer(40);
        if (!g_InvItemText[IT_QTY] && qty > 1) {
            sprintf(temp_text, "%d", qty);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }
        break;

    case O_KEY_OPTION_1:
    case O_KEY_OPTION_2:
    case O_KEY_OPTION_3:
    case O_KEY_OPTION_4:
    case O_LEADBAR_OPTION:
    case O_PICKUP_OPTION_1:
    case O_PICKUP_OPTION_2:
    case O_PUZZLE_OPTION_1:
    case O_PUZZLE_OPTION_2:
    case O_PUZZLE_OPTION_3:
    case O_PUZZLE_OPTION_4:
    case O_SCION_OPTION:
        if (!g_InvItemText[IT_QTY] && qty > 1) {
            sprintf(temp_text, "%d", qty);
            Overlay_MakeAmmoString(temp_text);
            g_InvItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(g_InvItemText[IT_QTY], 1);
            Text_CentreH(g_InvItemText[IT_QTY], 1);
        }

        show_examine_option = !Option_Examine_IsActive()
            && Option_Examine_CanExamine(inv_item->object_id);
        break;

    default:
        break;
    }

    if (inv_item->object_id == O_MEDI_OPTION
        || inv_item->object_id == O_BIGMEDI_OPTION) {
        if (g_Config.ui.healthbar_location == BL_TOP_LEFT) {
            Text_Hide(m_InvUpArrow1, true);
        } else if (g_Config.ui.healthbar_location == BL_TOP_RIGHT) {
            Text_Hide(m_InvUpArrow2, true);
        } else if (g_Config.ui.healthbar_location == BL_BOTTOM_LEFT) {
            Text_Hide(m_InvDownArrow1, true);
        } else if (g_Config.ui.healthbar_location == BL_BOTTOM_RIGHT) {
            Text_Hide(m_InvDownArrow2, true);
        }
        g_GameInfo.inv_showing_medpack = true;
    } else {
        Text_Hide(m_InvUpArrow1, false);
        Text_Hide(m_InvUpArrow2, false);
        Text_Hide(m_InvDownArrow1, false);
        Text_Hide(m_InvDownArrow2, false);
        g_GameInfo.inv_showing_medpack = false;
    }

    if (m_ExamineItemText != NULL) {
        Text_Hide(m_ExamineItemText, !show_examine_option);
        Text_Hide(m_UseItemText, !show_examine_option);
    }
}
