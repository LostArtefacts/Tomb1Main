#include "game/inventory_ring/control.h"

#include "game/console/common.h"
#include "game/game.h"
#include "game/game_string.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/inventory.h"
#include "game/inventory_ring/vars.h"
#include "game/lara/common.h"
#include "game/music.h"
#include "game/option.h"
#include "game/option/option_compass.h"
#include "game/option/option_examine.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/phase/phase.h"
#include "game/savegame.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/viewport.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/inventory_ring/priv.h>
#include <libtrx/game/math.h>
#include <libtrx/game/objects/names.h>
#include <libtrx/memory.h>

typedef enum {
    IT_NAME = 0,
    IT_QTY = 1,
    IT_NUMBER_OF = 2,
} INV_TEXT;

typedef enum {
    // clang-format off
    PASS_MESH_SPINE    = 1 << 0,
    PASS_MESH_FRONT    = 1 << 1,
    PASS_MESH_IN_FRONT = 1 << 2,
    PASS_MESH_PAGE_2   = 1 << 3,
    PASS_MESH_BACK     = 1 << 4,
    PASS_MESH_IN_BACK  = 1 << 5,
    PASS_MESH_PAGE_1   = 1 << 6,
    PASS_MESH_COMMON   = PASS_MESH_SPINE | PASS_MESH_BACK | PASS_MESH_FRONT,
    // clang-format on
} PASS_MESH;

static TEXTSTRING *m_UpArrow1 = NULL;
static TEXTSTRING *m_UpArrow2 = NULL;
static TEXTSTRING *m_DownArrow1 = NULL;
static TEXTSTRING *m_DownArrow2 = NULL;
static TEXTSTRING *m_ExamineItemText = NULL;
static TEXTSTRING *m_UseItemText = NULL;
static TEXTSTRING *m_VersionText = NULL;
static TEXTSTRING *m_HeadingText = NULL;
static TEXTSTRING *m_ItemText[IT_NUMBER_OF] = {};
static CLOCK_TIMER m_DemoTimer = { 0 };
static int32_t m_StartLevel;
static GAME_OBJECT_ID m_InvChosen;

static TEXTSTRING *M_InitExamineText(
    int32_t x_pos, const char *role_str, const char *input_str);
static void M_InitExamineOverlay(INV_RING *ring);
static void M_RemoveExamineOverlay(void);
static void M_RemoveItemsText(void);
static void M_RemoveVersionText(void);
static void M_InitHeader(INV_RING *ring);
static void M_RemoveHeader(void);
static void M_ShowItemQuantity(const char *fmt, int32_t qty);
static void M_ShowAmmoQuantity(const char *fmt, int32_t qty);

static void M_RingIsOpen(INV_RING *ring);
static void M_RingIsNotOpen(INV_RING *ring);
static void M_RingNotActive(const INVENTORY_ITEM *inv_item);
static void M_RingActive(INV_RING *ring);

static PHASE_CONTROL M_Control(INV_RING *ring);
static bool M_CheckDemoTimer(const INV_RING *ring);

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

static void M_InitExamineOverlay(INV_RING *const ring)
{
    if ((ring->mode != INV_GAME_MODE && ring->mode != INV_KEYS_MODE)
        || !g_Config.gameplay.enable_item_examining
        || m_ExamineItemText != NULL) {
        return;
    }

    m_ExamineItemText =
        M_InitExamineText(-100, GS(ITEM_EXAMINE_ROLE), GS(KEYMAP_LOOK));
    m_UseItemText =
        M_InitExamineText(100, GS(ITEM_USE_ROLE), GS(KEYMAP_ACTION));
}

static void M_RemoveExamineOverlay(void)
{
    if (m_ExamineItemText == NULL) {
        return;
    }

    Text_Remove(m_ExamineItemText);
    Text_Remove(m_UseItemText);
    m_ExamineItemText = NULL;
    m_UseItemText = NULL;
}

static void M_RemoveItemsText(void)
{
    for (int32_t i = 0; i < IT_NUMBER_OF; i++) {
        Text_Remove(m_ItemText[i]);
        m_ItemText[i] = NULL;
    }
}

static void M_RemoveVersionText(void)
{
    Text_Remove(m_VersionText);
    m_VersionText = NULL;
}

static void M_InitHeader(INV_RING *const ring)
{
    if (ring->mode == INV_TITLE_MODE) {
        return;
    }

    if (m_HeadingText == NULL) {
        switch (ring->type) {
        case RT_MAIN:
            m_HeadingText = Text_Create(0, 26, GS(HEADING_INVENTORY));
            break;

        case RT_OPTION:
            if (ring->mode == INV_DEATH_MODE) {
                m_HeadingText = Text_Create(0, 26, GS(HEADING_GAME_OVER));
            } else {
                m_HeadingText = Text_Create(0, 26, GS(HEADING_OPTION));
            }
            break;

        case RT_KEYS:
            m_HeadingText = Text_Create(0, 26, GS(HEADING_ITEMS));
            break;
        }

        Text_CentreH(m_HeadingText, true);
    }

    if (ring->mode != INV_GAME_MODE) {
        return;
    }

    if (m_UpArrow1 == NULL
        && (ring->type == RT_OPTION
            || (ring->type == RT_MAIN
                && g_InvRing_Source[RT_KEYS].count > 0))) {
        m_UpArrow1 = Text_Create(20, 28, "\\{arrow up}");
        m_UpArrow2 = Text_Create(-20, 28, "\\{arrow up}");
        Text_AlignRight(m_UpArrow2, true);
    }

    if (m_DownArrow1 == NULL
        && (ring->type == RT_MAIN || ring->type == RT_KEYS)) {
        m_DownArrow1 = Text_Create(20, -15, "\\{arrow down}");
        Text_AlignBottom(m_DownArrow1, true);
        m_DownArrow2 = Text_Create(-20, -15, "\\{arrow down}");
        Text_AlignBottom(m_DownArrow2, true);
        Text_AlignRight(m_DownArrow2, true);
    }
}

static void M_RemoveHeader(void)
{
    Text_Remove(m_HeadingText);
    m_HeadingText = NULL;
    Text_Remove(m_UpArrow1);
    m_UpArrow1 = NULL;
    Text_Remove(m_UpArrow2);
    m_UpArrow2 = NULL;
    Text_Remove(m_DownArrow1);
    m_DownArrow1 = NULL;
    Text_Remove(m_DownArrow2);
    m_DownArrow2 = NULL;
}

static void M_ShowItemQuantity(const char *const fmt, const int32_t qty)
{
    if (m_ItemText[IT_QTY] == NULL) {
        char string[64];
        sprintf(string, fmt, qty);
        Overlay_MakeAmmoString(string);
        m_ItemText[IT_QTY] = Text_Create(64, -56, string);
        Text_AlignBottom(m_ItemText[IT_QTY], true);
        Text_CentreH(m_ItemText[IT_QTY], true);
    }
}

static void M_ShowAmmoQuantity(const char *const fmt, const int32_t qty)
{
    if (!(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
        M_ShowItemQuantity(fmt, qty);
    }
}

static void M_RingIsOpen(INV_RING *const ring)
{
    M_InitHeader(ring);
    M_InitExamineOverlay(ring);
}

static void M_RingIsNotOpen(INV_RING *const ring)
{
    M_RemoveHeader();
    M_RemoveExamineOverlay();
}

static void M_RingNotActive(const INVENTORY_ITEM *const inv_item)
{
    if (m_ItemText[IT_NAME] == NULL
        && inv_item->object_id != O_PASSPORT_OPTION) {
        m_ItemText[IT_NAME] =
            Text_Create(0, -16, Object_GetName(inv_item->object_id));
        Text_AlignBottom(m_ItemText[IT_NAME], 1);
        Text_CentreH(m_ItemText[IT_NAME], 1);
    }

    const int32_t qty = Inv_RequestItem(inv_item->object_id);
    bool show_examine_option = false;

    switch (inv_item->object_id) {
    case O_SHOTGUN_OPTION:
        M_ShowAmmoQuantity("%5d A", g_Lara.shotgun.ammo / SHOTGUN_AMMO_CLIP);
        break;

    case O_MAGNUM_OPTION:
        M_ShowAmmoQuantity("%5d B", g_Lara.magnums.ammo);
        break;

    case O_UZI_OPTION:
        M_ShowAmmoQuantity("%5d C", g_Lara.uzis.ammo);
        break;

    case O_SG_AMMO_OPTION:
        M_ShowItemQuantity("%d", qty * NUM_SG_SHELLS);
        break;

    case O_MAG_AMMO_OPTION:
    case O_UZI_AMMO_OPTION:
        M_ShowItemQuantity("%d", qty * 2);
        break;

    case O_MEDI_OPTION:
    case O_BIGMEDI_OPTION:
        Overlay_BarSetHealthTimer(40);
        if (qty > 1) {
            M_ShowItemQuantity("%d", qty);
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
        if (qty > 1) {
            M_ShowItemQuantity("%d", qty);
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
            Text_Hide(m_UpArrow1, true);
        } else if (g_Config.ui.healthbar_location == BL_TOP_RIGHT) {
            Text_Hide(m_UpArrow2, true);
        } else if (g_Config.ui.healthbar_location == BL_BOTTOM_LEFT) {
            Text_Hide(m_DownArrow1, true);
        } else if (g_Config.ui.healthbar_location == BL_BOTTOM_RIGHT) {
            Text_Hide(m_DownArrow2, true);
        }
        g_GameInfo.inv_showing_medpack = true;
    } else {
        Text_Hide(m_UpArrow1, false);
        Text_Hide(m_UpArrow2, false);
        Text_Hide(m_DownArrow1, false);
        Text_Hide(m_DownArrow2, false);
        g_GameInfo.inv_showing_medpack = false;
    }

    if (m_ExamineItemText != NULL) {
        Text_Hide(m_ExamineItemText, !show_examine_option);
        Text_Hide(m_UseItemText, !show_examine_option);
    }
}

static void M_RingActive(INV_RING *const ring)
{
    InvRing_RemoveAllText();
}

static void M_SelectMeshes(INVENTORY_ITEM *const inv_item)
{
    switch (inv_item->object_id) {
    case O_PASSPORT_OPTION:
        inv_item->meshes_drawn = PASS_MESH_COMMON;
        if (inv_item->current_frame <= 14) {
            inv_item->meshes_drawn |= PASS_MESH_IN_FRONT | PASS_MESH_PAGE_1;
        } else if (inv_item->current_frame < 19) {
            inv_item->meshes_drawn |=
                PASS_MESH_IN_FRONT | PASS_MESH_PAGE_1 | PASS_MESH_PAGE_2;
        } else if (inv_item->current_frame == 19) {
            inv_item->meshes_drawn |= PASS_MESH_PAGE_1 | PASS_MESH_PAGE_2;
        } else if (inv_item->current_frame < 24) {
            inv_item->meshes_drawn |=
                PASS_MESH_PAGE_1 | PASS_MESH_PAGE_2 | PASS_MESH_IN_BACK;
        } else if (inv_item->current_frame < 29) {
            inv_item->meshes_drawn |= PASS_MESH_PAGE_2 | PASS_MESH_IN_BACK;
        } else if (inv_item->current_frame == 29) {
        }
        break;

    case O_COMPASS_OPTION:
        if (inv_item->current_frame == 0 || inv_item->current_frame >= 18) {
            inv_item->meshes_drawn = inv_item->meshes_sel;
        } else {
            inv_item->meshes_drawn = -1;
        }
        break;

    default:
        inv_item->meshes_drawn = -1;
        break;
    }
}

static bool M_AnimateInventoryItem(INVENTORY_ITEM *inv_item)
{
    if (inv_item->current_frame == inv_item->goal_frame) {
        M_SelectMeshes(inv_item);
        return false;
    }

    inv_item->current_frame += inv_item->anim_direction;
    if (inv_item->current_frame >= inv_item->frames_total) {
        inv_item->current_frame = 0;
    } else if (inv_item->current_frame < 0) {
        inv_item->current_frame = inv_item->frames_total - 1;
    }

    M_SelectMeshes(inv_item);
    return true;
}

// TODO: make this return a GAME_FLOW_COMMAND
static PHASE_CONTROL M_Control(INV_RING *const ring)
{
    if (ring->motion.status == RNG_OPENING) {
        if (ring->mode == INV_TITLE_MODE && Output_FadeIsAnimating()) {
            return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
        }

        Clock_ResetTimer(&m_DemoTimer);
        if (!ring->has_spun_out) {
            Sound_Effect(SFX_MENU_SPININ, NULL, SPM_ALWAYS);
            ring->has_spun_out = true;
        }
    }

    if (ring->motion.status == RNG_DONE) {
        // finish fading
        if (ring->mode == INV_TITLE_MODE) {
            Output_FadeToBlack(true);
        }

        if (Output_FadeIsAnimating()) {
            return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
        }

        return (PHASE_CONTROL) { .action = PHASE_ACTION_END };
    }

    InvRing_CalcAdders(ring, INV_RING_ROTATE_DURATION);

    Input_Update();
    // Do the demo inactivity check prior to postprocessing of the inputs.
    if (M_CheckDemoTimer(ring)) {
        ring->is_demo_needed = true;
    }
    Shell_ProcessInput();
    Game_ProcessInput();

    m_StartLevel = g_LevelComplete ? g_GameInfo.select_level_num : -1;

    if (g_IDelay) {
        if (g_IDCount) {
            g_IDCount--;
        } else {
            g_IDelay = false;
        }
    }

    g_GameInfo.inv_ring_above = ring->mode == INV_GAME_MODE
        && ((ring->type == RT_MAIN && g_InvRing_Source[RT_KEYS].count != 0)
            || (ring->type == RT_OPTION && g_InvRing_Source[RT_MAIN].count));

    if (ring->rotating) {
        return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
    }

    if ((ring->mode == INV_SAVE_MODE || ring->mode == INV_SAVE_CRYSTAL_MODE
         || ring->mode == INV_LOAD_MODE || ring->mode == INV_DEATH_MODE)
        && !ring->is_pass_open) {
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0, .menu_confirm = 1 };
    }

    if (!(ring->mode == INV_TITLE_MODE || Output_FadeIsAnimating()
          || ring->motion.status == RNG_OPENING)) {
        for (int i = 0; i < ring->number_of_objects; i++) {
            INVENTORY_ITEM *inv_item = ring->list[i];
            if (inv_item->object_id == O_COMPASS_OPTION) {
                Option_Compass_UpdateNeedle(inv_item);
            }
        }
    }

    switch (ring->motion.status) {
    case RNG_OPEN:
        if (g_Input.menu_right && ring->number_of_objects > 1) {
            InvRing_RotateLeft(ring);
            Sound_Effect(SFX_MENU_ROTATE, NULL, SPM_ALWAYS);
            break;
        }

        if (g_Input.menu_left && ring->number_of_objects > 1) {
            InvRing_RotateRight(ring);
            Sound_Effect(SFX_MENU_ROTATE, NULL, SPM_ALWAYS);
            break;
        }

        if (m_StartLevel != -1 || ring->is_demo_needed
            || (g_InputDB.menu_back && ring->mode != INV_TITLE_MODE)) {
            Sound_Effect(SFX_MENU_SPINOUT, NULL, SPM_ALWAYS);
            m_InvChosen = NO_OBJECT;

            if (ring->type == RT_MAIN) {
                g_InvRing_Source[RT_MAIN].current = ring->current_object;
            } else {
                g_InvRing_Source[RT_OPTION].current = ring->current_object;
            }

            if (ring->mode != INV_TITLE_MODE) {
                Output_FadeToTransparent(false);
            }

            InvRing_MotionSetup(ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
            InvRing_MotionRadius(ring, 0);
            InvRing_MotionCameraPos(ring, INV_RING_CAMERA_START_HEIGHT);
            InvRing_MotionRotation(
                ring, CLOSE_ROTATION, ring->ring_pos.rot.y - CLOSE_ROTATION);
            g_Input = (INPUT_STATE) { 0 };
            g_InputDB = (INPUT_STATE) { 0 };
        }

        const bool examine = g_InputDB.look && InvRing_CanExamine();
        if (g_InputDB.menu_confirm || examine) {
            if ((ring->mode == INV_SAVE_MODE
                 || ring->mode == INV_SAVE_CRYSTAL_MODE
                 || ring->mode == INV_LOAD_MODE || ring->mode == INV_DEATH_MODE)
                && !ring->is_pass_open) {
                ring->is_pass_open = true;
            }

            g_OptionSelected = 0;

            g_InvRing_Source[ring->type].current = ring->current_object;
            INVENTORY_ITEM *const inv_item =
                g_InvRing_Source[ring->type].items[ring->current_object];

            inv_item->goal_frame = inv_item->open_frame;
            inv_item->anim_direction = 1;
            inv_item->action = examine ? ACTION_EXAMINE : ACTION_USE;

            InvRing_MotionSetup(
                ring, RNG_SELECTING, RNG_SELECTED, SELECTING_FRAMES);
            InvRing_MotionRotation(
                ring, 0, -DEG_90 - ring->angle_adder * ring->current_object);
            InvRing_MotionItemSelect(ring, inv_item);
            g_Input = (INPUT_STATE) { 0 };
            g_InputDB = (INPUT_STATE) { 0 };

            switch (inv_item->object_id) {
            case O_COMPASS_OPTION:
                Sound_Effect(SFX_MENU_COMPASS, NULL, SPM_ALWAYS);
                break;

            case O_PHOTO_OPTION:
                Sound_Effect(SFX_MENU_CHOOSE, NULL, SPM_ALWAYS);
                break;

            case O_CONTROL_OPTION:
                Sound_Effect(SFX_MENU_GAMEBOY, NULL, SPM_ALWAYS);
                break;

            case O_PISTOL_OPTION:
            case O_SHOTGUN_OPTION:
            case O_MAGNUM_OPTION:
            case O_UZI_OPTION:
                Sound_Effect(SFX_MENU_GUNS, NULL, SPM_ALWAYS);
                break;

            default:
                Sound_Effect(SFX_MENU_SPININ, NULL, SPM_ALWAYS);
                break;
            }
        }

        if (g_InputDB.menu_up && ring->mode != INV_TITLE_MODE
            && ring->mode != INV_KEYS_MODE) {
            if (ring->type == RT_MAIN) {
                if (g_InvRing_Source[RT_KEYS].count != 0) {
                    InvRing_MotionSetup(
                        ring, RNG_CLOSING, RNG_MAIN2KEYS,
                        RINGSWITCH_FRAMES / 2);
                    InvRing_MotionRadius(ring, 0);
                    InvRing_MotionRotation(
                        ring, CLOSE_ROTATION,
                        ring->ring_pos.rot.y - CLOSE_ROTATION);
                    InvRing_MotionCameraPitch(ring, 0x2000);
                    ring->motion.misc = 0x2000;
                }
                g_Input = (INPUT_STATE) { 0 };
                g_InputDB = (INPUT_STATE) { 0 };
            } else if (ring->type == RT_OPTION) {
                if (g_InvRing_Source[RT_MAIN].count) {
                    InvRing_MotionSetup(
                        ring, RNG_CLOSING, RNG_OPTION2MAIN,
                        RINGSWITCH_FRAMES / 2);
                    InvRing_MotionRadius(ring, 0);
                    InvRing_MotionRotation(
                        ring, CLOSE_ROTATION,
                        ring->ring_pos.rot.y - CLOSE_ROTATION);
                    InvRing_MotionCameraPitch(ring, 0x2000);
                    ring->motion.misc = 0x2000;
                }
                g_InputDB = (INPUT_STATE) { 0 };
            }
        } else if (
            g_InputDB.menu_down && ring->mode != INV_TITLE_MODE
            && ring->mode != INV_KEYS_MODE) {
            if (ring->type == RT_KEYS) {
                if (g_InvRing_Source[RT_MAIN].count) {
                    InvRing_MotionSetup(
                        ring, RNG_CLOSING, RNG_KEYS2MAIN,
                        RINGSWITCH_FRAMES / 2);
                    InvRing_MotionRadius(ring, 0);
                    InvRing_MotionRotation(
                        ring, CLOSE_ROTATION,
                        ring->ring_pos.rot.y - CLOSE_ROTATION);
                    InvRing_MotionCameraPitch(ring, -0x2000);
                    ring->motion.misc = -0x2000;
                }
                g_Input = (INPUT_STATE) { 0 };
                g_InputDB = (INPUT_STATE) { 0 };
            } else if (ring->type == RT_MAIN) {
                if (g_InvRing_Source[RT_OPTION].count != 0) {
                    InvRing_MotionSetup(
                        ring, RNG_CLOSING, RNG_MAIN2OPTION,
                        RINGSWITCH_FRAMES / 2);
                    InvRing_MotionRadius(ring, 0);
                    InvRing_MotionRotation(
                        ring, CLOSE_ROTATION,
                        ring->ring_pos.rot.y - CLOSE_ROTATION);
                    InvRing_MotionCameraPitch(ring, -0x2000);
                    ring->motion.misc = -0x2000;
                }
                g_InputDB = (INPUT_STATE) { 0 };
            }
        }
        break;

    case RNG_MAIN2OPTION:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, INV_RING_RADIUS);
        ring->camera_pitch = -ring->motion.misc;
        ring->motion.camera_pitch_rate =
            ring->motion.misc / (RINGSWITCH_FRAMES / 2);
        ring->motion.camera_pitch_target = 0;
        g_InvRing_Source[RT_MAIN].current = ring->current_object;
        ring->type = RT_OPTION;
        ring->list = g_InvRing_Source[RT_OPTION].items;
        ring->number_of_objects = g_InvRing_Source[RT_OPTION].count;
        ring->current_object = g_InvRing_Source[RT_OPTION].current;
        InvRing_CalcAdders(ring, INV_RING_ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, INV_RING_OPEN_ROTATION,
            -DEG_90 - ring->angle_adder * ring->current_object);
        ring->ring_pos.rot.y =
            ring->motion.rotate_target + INV_RING_OPEN_ROTATION;
        break;

    case RNG_MAIN2KEYS:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, INV_RING_RADIUS);
        ring->camera_pitch = -ring->motion.misc;
        ring->motion.camera_pitch_rate =
            ring->motion.misc / (RINGSWITCH_FRAMES / 2);
        ring->motion.camera_pitch_target = 0;
        g_InvRing_Source[RT_MAIN].current = ring->current_object;
        g_InvRing_Source[RT_MAIN].count = ring->number_of_objects;
        ring->type = RT_KEYS;
        ring->list = g_InvRing_Source[RT_KEYS].items;
        ring->number_of_objects = g_InvRing_Source[RT_KEYS].count;
        ring->current_object = g_InvRing_Source[RT_KEYS].current;
        InvRing_CalcAdders(ring, INV_RING_ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, INV_RING_OPEN_ROTATION,
            -DEG_90 - ring->angle_adder * ring->current_object);
        ring->ring_pos.rot.y =
            ring->motion.rotate_target + INV_RING_OPEN_ROTATION;
        break;

    case RNG_KEYS2MAIN:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, INV_RING_RADIUS);
        ring->camera_pitch = -ring->motion.misc;
        ring->motion.camera_pitch_rate =
            ring->motion.misc / (RINGSWITCH_FRAMES / 2);
        ring->motion.camera_pitch_target = 0;
        g_InvRing_Source[RT_KEYS].current = ring->current_object;
        ring->type = RT_MAIN;
        ring->list = g_InvRing_Source[RT_MAIN].items;
        ring->number_of_objects = g_InvRing_Source[RT_MAIN].count;
        ring->current_object = g_InvRing_Source[RT_MAIN].current;
        InvRing_CalcAdders(ring, INV_RING_ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, INV_RING_OPEN_ROTATION,
            -DEG_90 - ring->angle_adder * ring->current_object);
        ring->ring_pos.rot.y =
            ring->motion.rotate_target + INV_RING_OPEN_ROTATION;
        break;

    case RNG_OPTION2MAIN:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, INV_RING_RADIUS);
        ring->camera_pitch = -ring->motion.misc;
        ring->motion.camera_pitch_rate =
            ring->motion.misc / (RINGSWITCH_FRAMES / 2);
        ring->motion.camera_pitch_target = 0;
        g_InvRing_Source[RT_OPTION].count = ring->number_of_objects;
        g_InvRing_Source[RT_OPTION].current = ring->current_object;
        ring->type = RT_MAIN;
        ring->list = g_InvRing_Source[RT_MAIN].items;
        ring->number_of_objects = g_InvRing_Source[RT_MAIN].count;
        ring->current_object = g_InvRing_Source[RT_MAIN].current;
        InvRing_CalcAdders(ring, INV_RING_ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, INV_RING_OPEN_ROTATION,
            -DEG_90 - ring->angle_adder * ring->current_object);
        ring->ring_pos.rot.y =
            ring->motion.rotate_target + INV_RING_OPEN_ROTATION;
        break;

    case RNG_SELECTED: {
        INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
        if (inv_item->object_id == O_PASSPORT_CLOSED) {
            inv_item->object_id = O_PASSPORT_OPTION;
        }

        bool busy = false;
        if (inv_item->y_rot == inv_item->y_rot_sel) {
            busy = M_AnimateInventoryItem(inv_item);
        }

        if (!busy && !g_IDelay) {
            Option_Control(inv_item);

            if (g_InputDB.menu_back) {
                inv_item->sprite_list = NULL;
                InvRing_MotionSetup(ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                g_Input = (INPUT_STATE) { 0 };
                g_InputDB = (INPUT_STATE) { 0 };

                if (ring->mode == INV_LOAD_MODE || ring->mode == INV_SAVE_MODE
                    || ring->mode == INV_SAVE_CRYSTAL_MODE) {
                    InvRing_MotionSetup(
                        ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                    g_Input = (INPUT_STATE) { 0 };
                    g_InputDB = (INPUT_STATE) { 0 };
                }
            }

            if (g_InputDB.menu_confirm) {
                inv_item->sprite_list = NULL;
                m_InvChosen = inv_item->object_id;
                if (ring->type == RT_MAIN) {
                    g_InvRing_Source[RT_MAIN].current = ring->current_object;
                } else {
                    g_InvRing_Source[RT_OPTION].current = ring->current_object;
                }

                if (ring->mode == INV_TITLE_MODE
                    && ((inv_item->object_id == O_DETAIL_OPTION)
                        || inv_item->object_id == O_SOUND_OPTION
                        || inv_item->object_id == O_CONTROL_OPTION
                        || inv_item->object_id == O_GAMMA_OPTION)) {
                    InvRing_MotionSetup(
                        ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                } else {
                    InvRing_MotionSetup(
                        ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                }
                g_Input = (INPUT_STATE) { 0 };
                g_InputDB = (INPUT_STATE) { 0 };
            }
        }
        break;
    }

    case RNG_DESELECT:
        Sound_Effect(SFX_MENU_SPINOUT, NULL, SPM_ALWAYS);
        InvRing_MotionSetup(ring, RNG_DESELECTING, RNG_OPEN, SELECTING_FRAMES);
        InvRing_MotionRotation(
            ring, 0, -DEG_90 - ring->angle_adder * ring->current_object);
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0 };
        break;

    case RNG_CLOSING_ITEM: {
        INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
        if (!M_AnimateInventoryItem(inv_item)) {
            if (inv_item->object_id == O_PASSPORT_OPTION) {
                inv_item->object_id = O_PASSPORT_CLOSED;
                inv_item->current_frame = 0;
            }
            ring->motion.count = SELECTING_FRAMES;
            ring->motion.status = ring->motion.status_target;
            InvRing_MotionItemDeselect(ring, inv_item);
            break;
        }
        break;
    }

    case RNG_EXITING_INVENTORY:
        if (ring->mode == INV_TITLE_MODE) {
        } else if (
            m_InvChosen == O_PASSPORT_OPTION
            && ((ring->mode == INV_LOAD_MODE && g_SavedGamesCount) /* f6 menu */
                || ring->mode == INV_DEATH_MODE /* Lara died */
                || (ring->mode == INV_GAME_MODE /* esc menu */
                    && g_GameInfo.passport_selection
                        != PASSPORT_MODE_SAVE_GAME /* but not save page */
                    )
                || g_CurrentLevel == g_GameFlow.gym_level_num /* Gym */
                || g_GameInfo.passport_selection == PASSPORT_MODE_RESTART)) {
            Output_FadeToBlack(false);
        } else {
            Output_FadeToTransparent(false);
        }

        if (!ring->motion.count) {
            InvRing_MotionSetup(ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
            InvRing_MotionRadius(ring, 0);
            InvRing_MotionCameraPos(ring, INV_RING_CAMERA_START_HEIGHT);
            InvRing_MotionRotation(
                ring, CLOSE_ROTATION, ring->ring_pos.rot.y - CLOSE_ROTATION);
        }
        break;

    default:
        break;
    }

    if (ring->motion.status == RNG_OPEN || ring->motion.status == RNG_SELECTING
        || ring->motion.status == RNG_SELECTED
        || ring->motion.status == RNG_DESELECTING
        || ring->motion.status == RNG_DESELECT
        || ring->motion.status == RNG_CLOSING_ITEM) {
        if (!ring->rotating && !g_Input.menu_left && !g_Input.menu_right) {
            INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
            M_RingNotActive(inv_item);
        }
        M_RingIsOpen(ring);
    } else {
        M_RingIsNotOpen(ring);
    }

    if (!ring->motion.status || ring->motion.status == RNG_CLOSING
        || ring->motion.status == RNG_MAIN2OPTION
        || ring->motion.status == RNG_OPTION2MAIN
        || ring->motion.status == RNG_EXITING_INVENTORY
        || ring->motion.status == RNG_DONE || ring->rotating) {
        M_RingActive(ring);
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static bool M_CheckDemoTimer(const INV_RING *const ring)
{
    if (!g_Config.gameplay.enable_demo || !g_GameFlow.has_demo) {
        return false;
    }

    if (ring->mode != INV_TITLE_MODE || g_Input.any || g_InputDB.any
        || Console_IsOpened()) {
        Clock_ResetTimer(&m_DemoTimer);
        return false;
    }

    return ring->motion.status == RNG_OPEN
        && Clock_CheckElapsedMilliseconds(
               &m_DemoTimer, g_GameFlow.demo_delay * 1000.0);
}

bool InvRing_CanExamine(void)
{
    return g_Config.gameplay.enable_item_examining && m_ExamineItemText != NULL
        && !m_ExamineItemText->flags.hide;
}

void InvRing_RemoveAllText(void)
{
    M_RemoveHeader();
    M_RemoveItemsText();
    M_RemoveExamineOverlay();
}

INV_RING *InvRing_Open(const INVENTORY_MODE mode)
{
    g_PhdLeft = Viewport_GetMinX();
    g_PhdTop = Viewport_GetMinY();
    g_PhdBottom = Viewport_GetMaxY();
    g_PhdRight = Viewport_GetMaxX();
    m_InvChosen = NO_OBJECT;

    g_InvRing_OldCamera = g_Camera;
    m_StartLevel = -1;

    if (mode == INV_TITLE_MODE) {
        g_InvRing_Source[RT_OPTION].count = TITLE_RING_OBJECTS;
        m_VersionText = Text_Create(-20, -18, g_TR1XVersion);
        Text_AlignRight(m_VersionText, 1);
        Text_AlignBottom(m_VersionText, 1);
        Text_SetScale(
            m_VersionText, TEXT_BASE_SCALE * 0.5, TEXT_BASE_SCALE * 0.5);
    } else {
        g_InvRing_Source[RT_OPTION].count = OPTION_RING_OBJECTS;
        M_RemoveVersionText();
    }

    g_InvRing_Source[RT_MAIN].current = 0;
    for (int32_t i = 0; i < g_InvRing_Source[RT_MAIN].count; i++) {
        InvRing_InitInvItem(g_InvRing_Source[RT_MAIN].items[i]);
    }
    g_InvRing_Source[RT_OPTION].current = 0;
    for (int32_t i = 0; i < g_InvRing_Source[RT_OPTION].count; i++) {
        InvRing_InitInvItem(g_InvRing_Source[RT_OPTION].items[i]);
    }

    g_OptionSelected = 0;

    if (g_GameFlow.gym_level_num == -1) {
        Inv_RemoveItem(O_PHOTO_OPTION);
    }

    if (!g_Config.audio.enable_music_in_inventory && mode != INV_TITLE_MODE) {
        Music_Pause();
        Sound_PauseAll();
    } else {
        Sound_ResetAmbient();
        Sound_UpdateEffects();
    }

    INV_RING *const ring = Memory_Alloc(sizeof(INV_RING));
    ring->mode = mode;

    switch (mode) {
    case INV_DEATH_MODE:
    case INV_SAVE_MODE:
    case INV_SAVE_CRYSTAL_MODE:
    case INV_LOAD_MODE:
    case INV_TITLE_MODE:
        InvRing_InitRing(
            ring, RT_OPTION, g_InvRing_Source[RT_OPTION].items,
            g_InvRing_Source[RT_OPTION].count,
            g_InvRing_Source[RT_OPTION].current);
        break;

    case INV_KEYS_MODE:
        InvRing_InitRing(
            ring, RT_KEYS, g_InvRing_Source[RT_KEYS].items,
            g_InvRing_Source[RT_KEYS].count, g_InvRing_Source[RT_MAIN].current);
        break;

    default:
        if (g_InvRing_Source[RT_MAIN].count != 0) {
            InvRing_InitRing(
                ring, RT_MAIN, g_InvRing_Source[RT_MAIN].items,
                g_InvRing_Source[RT_MAIN].count,
                g_InvRing_Source[RT_MAIN].current);
        } else {
            InvRing_InitRing(
                ring, RT_OPTION, g_InvRing_Source[RT_OPTION].items,
                g_InvRing_Source[RT_OPTION].count,
                g_InvRing_Source[RT_OPTION].current);
        }
        break;
    }

    // reset the delta timer before starting the spinout animation
    Clock_ResetTimer(&g_InvRing_MotionTimer);

    if (mode == INV_TITLE_MODE) {
        Output_LoadBackgroundFromFile(g_GameFlow.main_menu_background_path);
    } else {
        Output_UnloadBackground();
    }

    return ring;
}

PHASE_CONTROL InvRing_Close(INV_RING *const ring)
{
    PHASE_CONTROL result = { .action = PHASE_ACTION_NO_WAIT };

    InvRing_RemoveAllText();
    M_RemoveVersionText();

    if (ring->list != NULL) {
        INVENTORY_ITEM *const inv_item = ring->list[ring->current_object];
        if (inv_item != NULL) {
            Option_Shutdown(inv_item);
        }
    }
    if (ring->mode == INV_TITLE_MODE) {
        Music_Stop();
        Sound_StopAllSamples();
    }
    Output_UnloadBackground();

    if (g_Config.gameplay.fix_item_duplication_glitch) {
        Inv_ClearSelection();
    }
    if (g_Config.input.enable_buffering) {
        g_OldInputDB = (INPUT_STATE) { 0 };
    }

    if (m_StartLevel != -1) {
        result = (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = {
                .action = GF_SELECT_GAME,
                .param = m_StartLevel,
            },
        };
        goto finish;
    }

    if (ring->is_demo_needed) {
        result = (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_START_DEMO, .param = -1 },
        };
        goto finish;
    }

    switch (m_InvChosen) {
    case O_PASSPORT_OPTION:
        switch (g_GameInfo.passport_selection) {
        case PASSPORT_MODE_LOAD_GAME:
            result = (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_START_SAVED_GAME,
                    .param = g_GameInfo.current_save_slot,
                },
            };
            goto finish;

        case PASSPORT_MODE_SELECT_LEVEL:
            result = (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_SELECT_GAME,
                    .param = g_GameInfo.select_level_num,
                },
            };
            goto finish;

        case PASSPORT_MODE_STORY_SO_FAR:
            result = (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_STORY_SO_FAR,
                    .param = g_GameInfo.current_save_slot,
                },
            };
            goto finish;

        case PASSPORT_MODE_NEW_GAME:
            Savegame_InitCurrentInfo();
            result = (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_START_GAME,
                    .param = g_GameFlow.first_level_num,
                },
            };
            goto finish;

        case PASSPORT_MODE_SAVE_GAME:
            Savegame_Save(g_GameInfo.current_save_slot);
            Music_Unpause();
            Sound_UnpauseAll();
            Phase_Set(PHASE_GAME, NULL);
            result = (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
            goto finish;

        case PASSPORT_MODE_RESTART:
            result = (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_RESTART_GAME,
                    .param = g_CurrentLevel,
                },
            };
            goto finish;

        case PASSPORT_MODE_EXIT_TITLE:
            result = (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_EXIT_TO_TITLE },
            };
            goto finish;

        case PASSPORT_MODE_EXIT_GAME:
            result = (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_EXIT_GAME },
            };
            goto finish;

        case PASSPORT_MODE_BROWSE:
        case PASSPORT_MODE_UNAVAILABLE:
        default:
            result = (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_EXIT_TO_TITLE },
            };
            goto finish;
        }

    case O_PHOTO_OPTION:
        g_GameInfo.current_save_slot = -1;
        result = (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = {
                .action = GF_START_GYM,
                .param = g_GameFlow.gym_level_num,
            },
        };
        goto finish;

    case O_PISTOL_OPTION:
    case O_SHOTGUN_OPTION:
    case O_MAGNUM_OPTION:
    case O_UZI_OPTION:
    case O_MEDI_OPTION:
    case O_BIGMEDI_OPTION:
    case O_KEY_OPTION_1:
    case O_KEY_OPTION_2:
    case O_KEY_OPTION_3:
    case O_KEY_OPTION_4:
    case O_PUZZLE_OPTION_1:
    case O_PUZZLE_OPTION_2:
    case O_PUZZLE_OPTION_3:
    case O_PUZZLE_OPTION_4:
    case O_LEADBAR_OPTION:
    case O_SCION_OPTION:
        Lara_UseItem(m_InvChosen);
        break;

    default:
        break;
    }

    if (ring->mode == INV_TITLE_MODE) {
        result = (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_NOOP },
        };
        goto finish;
    } else {
        Music_Unpause();
        Sound_UnpauseAll();
        Phase_Set(PHASE_GAME, NULL);
        result = (PHASE_CONTROL) { .action = PHASE_ACTION_NO_WAIT };
        goto finish;
    }

finish:
    m_InvChosen = NO_OBJECT;
    Memory_Free(ring);
    return result;
}

PHASE_CONTROL InvRing_Control(INV_RING *const ring, const int32_t num_frames)
{
    for (int32_t i = 0; i < num_frames; i++) {
        const PHASE_CONTROL result = M_Control(ring);
        if (result.action == PHASE_ACTION_END) {
            return result;
        }
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}
