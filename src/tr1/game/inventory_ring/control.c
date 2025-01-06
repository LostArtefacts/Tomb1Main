#include "game/inventory_ring/control.h"

#include "game/console/common.h"
#include "game/game.h"
#include "game/game_string.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/inventory.h"
#include "game/inventory_ring/priv.h"
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
#include <libtrx/game/math.h>
#include <libtrx/game/objects/names.h>

typedef enum {
    PSPINE = 1,
    PFRONT = 2,
    PINFRONT = 4,
    PPAGE2 = 8,
    PBACK = 16,
    PINBACK = 32,
    PPAGE1 = 64
} PASS_PAGE;

static TEXTSTRING *m_InvDownArrow1 = NULL;
static TEXTSTRING *m_InvDownArrow2 = NULL;
static TEXTSTRING *m_InvUpArrow1 = NULL;
static TEXTSTRING *m_InvUpArrow2 = NULL;
static TEXTSTRING *m_ExamineItemText = NULL;
static TEXTSTRING *m_UseItemText = NULL;
static TEXTSTRING *m_VersionText = NULL;
static TEXTSTRING *m_ItemText[IT_NUMBER_OF] = {};
static TEXTSTRING *m_HeadingText;
static CLOCK_TIMER m_DemoTimer = { 0 };
static int32_t m_StartLevel;
static GAME_OBJECT_ID m_InvChosen;

static TEXTSTRING *M_InitExamineText(
    int32_t x_pos, const char *role_str, const char *input_str);
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

// TODO: make this return a GAME_FLOW_COMMAND
static PHASE_CONTROL M_Control(INV_RING *const ring)
{
    if (ring->motion.status == RNG_OPENING) {
        if (g_InvMode == INV_TITLE_MODE && Output_FadeIsAnimating()) {
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
        if (g_InvMode == INV_TITLE_MODE) {
            Output_FadeToBlack(true);
        }

        if (Output_FadeIsAnimating()) {
            return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
        }

        return InvRing_Close(ring, m_InvChosen);
    }

    InvRing_CalcAdders(ring, ROTATE_DURATION);

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

    g_GameInfo.inv_ring_above = g_InvMode == INV_GAME_MODE
        && ((ring->type == RT_MAIN && g_InvKeysObjects)
            || (ring->type == RT_OPTION && g_InvMainObjects));

    if (ring->rotating) {
        return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
    }

    if ((g_InvMode == INV_SAVE_MODE || g_InvMode == INV_SAVE_CRYSTAL_MODE
         || g_InvMode == INV_LOAD_MODE || g_InvMode == INV_DEATH_MODE)
        && !ring->is_pass_open) {
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0, .menu_confirm = 1 };
    }

    if (!(g_InvMode == INV_TITLE_MODE || Output_FadeIsAnimating()
          || ring->motion.status == RNG_OPENING)) {
        for (int i = 0; i < ring->number_of_objects; i++) {
            INVENTORY_ITEM *inv_item = ring->list[i];
            if (inv_item->object_id == O_MAP_OPTION) {
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
            || (g_InputDB.menu_back && g_InvMode != INV_TITLE_MODE)) {
            Sound_Effect(SFX_MENU_SPINOUT, NULL, SPM_ALWAYS);
            m_InvChosen = NO_OBJECT;

            if (ring->type == RT_MAIN) {
                g_InvMainCurrent = ring->current_object;
            } else {
                g_InvOptionCurrent = ring->current_object;
            }

            if (g_InvMode != INV_TITLE_MODE) {
                Output_FadeToTransparent(false);
            }

            InvRing_MotionSetup(ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
            InvRing_MotionRadius(ring, 0);
            InvRing_MotionCameraPos(ring, CAMERA_STARTHEIGHT);
            InvRing_MotionRotation(
                ring, CLOSE_ROTATION, ring->ring_pos.rot.y - CLOSE_ROTATION);
            g_Input = (INPUT_STATE) { 0 };
            g_InputDB = (INPUT_STATE) { 0 };
        }

        const bool examine = g_InputDB.look && InvRing_CanExamine();
        if (g_InputDB.menu_confirm || examine) {
            if ((g_InvMode == INV_SAVE_MODE
                 || g_InvMode == INV_SAVE_CRYSTAL_MODE
                 || g_InvMode == INV_LOAD_MODE || g_InvMode == INV_DEATH_MODE)
                && !ring->is_pass_open) {
                ring->is_pass_open = true;
            }

            g_OptionSelected = 0;

            INVENTORY_ITEM *inv_item;
            if (ring->type == RT_MAIN) {
                g_InvMainCurrent = ring->current_object;
                inv_item = g_InvMainList[ring->current_object];
            } else if (ring->type == RT_OPTION) {
                g_InvOptionCurrent = ring->current_object;
                inv_item = g_InvOptionList[ring->current_object];
            } else {
                g_InvKeysCurrent = ring->current_object;
                inv_item = g_InvKeysList[ring->current_object];
            }

            inv_item->goal_frame = inv_item->open_frame;
            inv_item->anim_direction = 1;
            inv_item->action = examine ? ACTION_EXAMINE : ACTION_USE;

            InvRing_MotionSetup(
                ring, RNG_SELECTING, RNG_SELECTED, SELECTING_FRAMES);
            InvRing_MotionRotation(
                ring, 0, -PHD_90 - ring->angle_adder * ring->current_object);
            InvRing_MotionItemSelect(ring, inv_item);
            g_Input = (INPUT_STATE) { 0 };
            g_InputDB = (INPUT_STATE) { 0 };

            switch (inv_item->object_id) {
            case O_MAP_OPTION:
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

        if (g_InputDB.menu_up && g_InvMode != INV_TITLE_MODE
            && g_InvMode != INV_KEYS_MODE) {
            if (ring->type == RT_MAIN) {
                if (g_InvKeysObjects) {
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
                if (g_InvMainObjects) {
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
            g_InputDB.menu_down && g_InvMode != INV_TITLE_MODE
            && g_InvMode != INV_KEYS_MODE) {
            if (ring->type == RT_KEYS) {
                if (g_InvMainObjects) {
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
                if (g_InvOptionObjects) {
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
        InvRing_MotionRadius(ring, RING_RADIUS);
        ring->camera_pitch = -ring->motion.misc;
        ring->motion.camera_pitch_rate =
            ring->motion.misc / (RINGSWITCH_FRAMES / 2);
        ring->motion.camera_pitch_target = 0;
        g_InvMainCurrent = ring->current_object;
        ring->list = g_InvOptionList;
        ring->type = RT_OPTION;
        ring->number_of_objects = g_InvOptionObjects;
        ring->current_object = g_InvOptionCurrent;
        InvRing_CalcAdders(ring, ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, OPEN_ROTATION,
            -PHD_90 - ring->angle_adder * ring->current_object);
        ring->ring_pos.rot.y = ring->motion.rotate_target + OPEN_ROTATION;
        break;

    case RNG_MAIN2KEYS:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, RING_RADIUS);
        ring->camera_pitch = -ring->motion.misc;
        ring->motion.camera_pitch_rate =
            ring->motion.misc / (RINGSWITCH_FRAMES / 2);
        ring->motion.camera_pitch_target = 0;
        g_InvMainCurrent = ring->current_object;
        g_InvMainObjects = ring->number_of_objects;
        ring->list = g_InvKeysList;
        ring->type = RT_KEYS;
        ring->number_of_objects = g_InvKeysObjects;
        ring->current_object = g_InvKeysCurrent;
        InvRing_CalcAdders(ring, ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, OPEN_ROTATION,
            -PHD_90 - ring->angle_adder * ring->current_object);
        ring->ring_pos.rot.y = ring->motion.rotate_target + OPEN_ROTATION;
        break;

    case RNG_KEYS2MAIN:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, RING_RADIUS);
        ring->camera_pitch = -ring->motion.misc;
        ring->motion.camera_pitch_rate =
            ring->motion.misc / (RINGSWITCH_FRAMES / 2);
        ring->motion.camera_pitch_target = 0;
        g_InvKeysCurrent = ring->current_object;
        ring->list = g_InvMainList;
        ring->type = RT_MAIN;
        ring->number_of_objects = g_InvMainObjects;
        ring->current_object = g_InvMainCurrent;
        InvRing_CalcAdders(ring, ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, OPEN_ROTATION,
            -PHD_90 - ring->angle_adder * ring->current_object);
        ring->ring_pos.rot.y = ring->motion.rotate_target + OPEN_ROTATION;
        break;

    case RNG_OPTION2MAIN:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, RING_RADIUS);
        ring->camera_pitch = -ring->motion.misc;
        ring->motion.camera_pitch_rate =
            ring->motion.misc / (RINGSWITCH_FRAMES / 2);
        ring->motion.camera_pitch_target = 0;
        g_InvOptionObjects = ring->number_of_objects;
        g_InvOptionCurrent = ring->current_object;
        ring->list = g_InvMainList;
        ring->type = RT_MAIN;
        ring->number_of_objects = g_InvMainObjects;
        ring->current_object = g_InvMainCurrent;
        InvRing_CalcAdders(ring, ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, OPEN_ROTATION,
            -PHD_90 - ring->angle_adder * ring->current_object);
        ring->ring_pos.rot.y = ring->motion.rotate_target + OPEN_ROTATION;
        break;

    case RNG_SELECTED: {
        INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
        if (inv_item->object_id == O_PASSPORT_CLOSED) {
            inv_item->object_id = O_PASSPORT_OPTION;
        }

        bool busy = false;
        if (inv_item->y_rot == inv_item->y_rot_sel) {
            busy = InvRing_AnimateItem(inv_item);
        }

        if (!busy && !g_IDelay) {
            Option_Control(inv_item);

            if (g_InputDB.menu_back) {
                inv_item->sprite_list = NULL;
                InvRing_MotionSetup(ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                g_Input = (INPUT_STATE) { 0 };
                g_InputDB = (INPUT_STATE) { 0 };

                if (g_InvMode == INV_LOAD_MODE || g_InvMode == INV_SAVE_MODE
                    || g_InvMode == INV_SAVE_CRYSTAL_MODE) {
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
                    g_InvMainCurrent = ring->current_object;
                } else {
                    g_InvOptionCurrent = ring->current_object;
                }

                if (g_InvMode == INV_TITLE_MODE
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
            ring, 0, -PHD_90 - ring->angle_adder * ring->current_object);
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0 };
        break;

    case RNG_CLOSING_ITEM: {
        INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
        if (!InvRing_AnimateItem(inv_item)) {
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
        if (g_InvMode == INV_TITLE_MODE) {
        } else if (
            m_InvChosen == O_PASSPORT_OPTION
            && ((g_InvMode == INV_LOAD_MODE && g_SavedGamesCount) /* f6 menu */
                || g_InvMode == INV_DEATH_MODE /* Lara died */
                || (g_InvMode == INV_GAME_MODE /* esc menu */
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
            InvRing_MotionCameraPos(ring, CAMERA_STARTHEIGHT);
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
            InvRing_Active(inv_item);
        }
        InvRing_InitHeader(ring);
        InvRing_InitExamineOverlay();
    } else {
        InvRing_RemoveHeader();
        InvRing_RemoveExamineOverlay();
    }

    if (!ring->motion.status || ring->motion.status == RNG_CLOSING
        || ring->motion.status == RNG_MAIN2OPTION
        || ring->motion.status == RNG_OPTION2MAIN
        || ring->motion.status == RNG_EXITING_INVENTORY
        || ring->motion.status == RNG_DONE || ring->rotating) {
        InvRing_RemoveAllText();
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static bool M_CheckDemoTimer(const INV_RING *const ring)
{
    if (!g_Config.gameplay.enable_demo || !g_GameFlow.has_demo) {
        return false;
    }

    if (g_InvMode != INV_TITLE_MODE || g_Input.any || g_InputDB.any
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

void InvRing_InitHeader(INV_RING *ring)
{
    if (g_InvMode == INV_TITLE_MODE) {
        return;
    }

    if (!m_HeadingText) {
        switch (ring->type) {
        case RT_MAIN:
            m_HeadingText = Text_Create(0, 26, GS(HEADING_INVENTORY));
            break;

        case RT_OPTION:
            if (g_InvMode == INV_DEATH_MODE) {
                m_HeadingText = Text_Create(0, 26, GS(HEADING_GAME_OVER));
            } else {
                m_HeadingText = Text_Create(0, 26, GS(HEADING_OPTION));
            }
            break;

        case RT_KEYS:
            m_HeadingText = Text_Create(0, 26, GS(HEADING_ITEMS));
            break;
        }

        Text_CentreH(m_HeadingText, 1);
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
    if (m_HeadingText != NULL) {
        Text_Remove(m_HeadingText);
        m_HeadingText = NULL;
    }
    if (m_InvUpArrow1 != NULL) {
        Text_Remove(m_InvUpArrow1);
        m_InvUpArrow1 = NULL;
    }
    if (m_InvUpArrow2 != NULL) {
        Text_Remove(m_InvUpArrow2);
        m_InvUpArrow2 = NULL;
    }
    if (m_InvDownArrow1 != NULL) {
        Text_Remove(m_InvDownArrow1);
        m_InvDownArrow1 = NULL;
    }
    if (m_InvDownArrow2 != NULL) {
        Text_Remove(m_InvDownArrow2);
        m_InvDownArrow2 = NULL;
    }
}

void InvRing_RemoveAllText(void)
{
    InvRing_RemoveHeader();
    InvRing_RemoveExamineOverlay();
    for (int i = 0; i < IT_NUMBER_OF; i++) {
        if (m_ItemText[i]) {
            Text_Remove(m_ItemText[i]);
            m_ItemText[i] = NULL;
        }
    }
}

void InvRing_Active(INVENTORY_ITEM *inv_item)
{
    if (m_ItemText[IT_NAME] == NULL
        && inv_item->object_id != O_PASSPORT_OPTION) {
        m_ItemText[IT_NAME] =
            Text_Create(0, -16, Object_GetName(inv_item->object_id));
        Text_AlignBottom(m_ItemText[IT_NAME], 1);
        Text_CentreH(m_ItemText[IT_NAME], 1);
    }

    char temp_text[128];
    int32_t qty = Inv_RequestItem(inv_item->object_id);

    bool show_examine_option = false;

    switch (inv_item->object_id) {
    case O_SHOTGUN_OPTION:
        if (!m_ItemText[IT_QTY] && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
            sprintf(
                temp_text, "%5d A", g_Lara.shotgun.ammo / SHOTGUN_AMMO_CLIP);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
        }
        break;

    case O_MAGNUM_OPTION:
        if (!m_ItemText[IT_QTY] && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
            sprintf(temp_text, "%5d B", g_Lara.magnums.ammo);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
        }
        break;

    case O_UZI_OPTION:
        if (!m_ItemText[IT_QTY] && !(g_GameInfo.bonus_flag & GBF_NGPLUS)) {
            sprintf(temp_text, "%5d C", g_Lara.uzis.ammo);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
        }
        break;

    case O_SG_AMMO_OPTION:
        if (!m_ItemText[IT_QTY]) {
            sprintf(temp_text, "%d", qty * NUM_SG_SHELLS);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
        }
        break;

    case O_MAG_AMMO_OPTION:
        if (!m_ItemText[IT_QTY]) {
            sprintf(temp_text, "%d", Inv_RequestItem(O_MAG_AMMO_OPTION) * 2);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
        }
        break;

    case O_UZI_AMMO_OPTION:
        if (!m_ItemText[IT_QTY]) {
            sprintf(temp_text, "%d", Inv_RequestItem(O_UZI_AMMO_OPTION) * 2);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
        }
        break;

    case O_MEDI_OPTION:
        Overlay_BarSetHealthTimer(40);
        if (!m_ItemText[IT_QTY] && qty > 1) {
            sprintf(temp_text, "%d", qty);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
        }
        break;

    case O_BIGMEDI_OPTION:
        Overlay_BarSetHealthTimer(40);
        if (!m_ItemText[IT_QTY] && qty > 1) {
            sprintf(temp_text, "%d", qty);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
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
        if (!m_ItemText[IT_QTY] && qty > 1) {
            sprintf(temp_text, "%d", qty);
            Overlay_MakeAmmoString(temp_text);
            m_ItemText[IT_QTY] = Text_Create(64, -56, temp_text);
            Text_AlignBottom(m_ItemText[IT_QTY], 1);
            Text_CentreH(m_ItemText[IT_QTY], 1);
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

void InvRing_Construct(void)
{
    g_PhdLeft = Viewport_GetMinX();
    g_PhdTop = Viewport_GetMinY();
    g_PhdBottom = Viewport_GetMaxY();
    g_PhdRight = Viewport_GetMaxX();
    m_InvChosen = NO_OBJECT;

    g_InvRing_OldCamera = g_Camera;
    m_StartLevel = -1;

    if (g_InvMode == INV_TITLE_MODE) {
        g_InvOptionObjects = TITLE_RING_OBJECTS;
        m_VersionText = Text_Create(-20, -18, g_TR1XVersion);
        Text_AlignRight(m_VersionText, 1);
        Text_AlignBottom(m_VersionText, 1);
        Text_SetScale(
            m_VersionText, TEXT_BASE_SCALE * 0.5, TEXT_BASE_SCALE * 0.5);
    } else {
        g_InvOptionObjects = OPTION_RING_OBJECTS;
        Text_Remove(m_VersionText);
        m_VersionText = NULL;
    }

    for (int i = 0; i < g_InvMainObjects; i++) {
        InvRing_ResetItem(g_InvMainList[i]);
    }

    for (int i = 0; i < g_InvOptionObjects; i++) {
        InvRing_ResetItem(g_InvOptionList[i]);
    }

    g_InvMainCurrent = 0;
    g_InvOptionCurrent = 0;
    g_OptionSelected = 0;

    if (g_GameFlow.gym_level_num == -1) {
        Inv_RemoveItem(O_PHOTO_OPTION);
    }

    // reset the delta timer before starting the spinout animation
    Clock_ResetTimer(&g_InvRing_MotionTimer);
}

void InvRing_Destroy(void)
{
    InvRing_RemoveAllText();
    m_InvChosen = NO_OBJECT;

    if (g_Config.gameplay.fix_item_duplication_glitch) {
        Inv_ClearSelection();
    }

    if (m_VersionText) {
        Text_Remove(m_VersionText);
        m_VersionText = NULL;
    }
}

PHASE_CONTROL InvRing_Close(INV_RING *const ring, GAME_OBJECT_ID inv_chosen)
{
    const bool is_demo_needed = ring->is_demo_needed;
    InvRing_Destroy();

    if (m_StartLevel != -1) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = {
                .action = GF_SELECT_GAME,
                .param = m_StartLevel,
            },
        };
    }

    if (is_demo_needed) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_START_DEMO, .param = -1 },
        };
    }

    switch (inv_chosen) {
    case O_PASSPORT_OPTION:
        switch (g_GameInfo.passport_selection) {
        case PASSPORT_MODE_LOAD_GAME:
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_START_SAVED_GAME,
                    .param = g_GameInfo.current_save_slot,
                },
            };

        case PASSPORT_MODE_SELECT_LEVEL:
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_SELECT_GAME,
                    .param = g_GameInfo.select_level_num,
                },
            };

        case PASSPORT_MODE_STORY_SO_FAR:
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_STORY_SO_FAR,
                    .param = g_GameInfo.current_save_slot,
                },
            };

        case PASSPORT_MODE_NEW_GAME:
            Savegame_InitCurrentInfo();
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_START_GAME,
                    .param = g_GameFlow.first_level_num,
                },
            };

        case PASSPORT_MODE_SAVE_GAME:
            Savegame_Save(g_GameInfo.current_save_slot);
            Music_Unpause();
            Sound_UnpauseAll();
            Phase_Set(PHASE_GAME, NULL);
            return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };

        case PASSPORT_MODE_RESTART:
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = {
                    .action = GF_RESTART_GAME,
                    .param = g_CurrentLevel,
                },
            };

        case PASSPORT_MODE_EXIT_TITLE:
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_EXIT_TO_TITLE },
            };

        case PASSPORT_MODE_EXIT_GAME:
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_EXIT_GAME },
            };

        case PASSPORT_MODE_BROWSE:
        case PASSPORT_MODE_UNAVAILABLE:
        default:
            return (PHASE_CONTROL) {
                .action = PHASE_ACTION_END,
                .gf_cmd = { .action = GF_EXIT_TO_TITLE },
            };
        }

    case O_PHOTO_OPTION:
        g_GameInfo.current_save_slot = -1;
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = {
                .action = GF_START_GYM,
                .param = g_GameFlow.gym_level_num,
            },
        };

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
        Lara_UseItem(inv_chosen);
        break;

    default:
        break;
    }

    if (g_InvMode == INV_TITLE_MODE) {
        return (PHASE_CONTROL) {
            .action = PHASE_ACTION_END,
            .gf_cmd = { .action = GF_NOOP },
        };
    } else {
        Music_Unpause();
        Sound_UnpauseAll();
        Phase_Set(PHASE_GAME, NULL);
        return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
    }
}

void InvRing_SelectMeshes(INVENTORY_ITEM *inv_item)
{
    if (inv_item->object_id == O_PASSPORT_OPTION) {
        if (inv_item->current_frame <= 14) {
            inv_item->meshes_drawn = PASS_MESH | PINFRONT | PPAGE1;
        } else if (inv_item->current_frame < 19) {
            inv_item->meshes_drawn = PASS_MESH | PINFRONT | PPAGE1 | PPAGE2;
        } else if (inv_item->current_frame == 19) {
            inv_item->meshes_drawn = PASS_MESH | PPAGE1 | PPAGE2;
        } else if (inv_item->current_frame < 24) {
            inv_item->meshes_drawn = PASS_MESH | PPAGE1 | PPAGE2 | PINBACK;
        } else if (inv_item->current_frame < 29) {
            inv_item->meshes_drawn = PASS_MESH | PPAGE2 | PINBACK;
        } else if (inv_item->current_frame == 29) {
            inv_item->meshes_drawn = PASS_MESH;
        }
    } else if (inv_item->object_id == O_MAP_OPTION) {
        if (inv_item->current_frame && inv_item->current_frame < 18) {
            inv_item->meshes_drawn = -1;
        } else {
            inv_item->meshes_drawn = inv_item->meshes_sel;
        }
    } else {
        inv_item->meshes_drawn = -1;
    }
}

bool InvRing_AnimateItem(INVENTORY_ITEM *inv_item)
{
    if (inv_item->current_frame == inv_item->goal_frame) {
        InvRing_SelectMeshes(inv_item);
        return false;
    }

    inv_item->current_frame += inv_item->anim_direction;
    if (inv_item->current_frame >= inv_item->frames_total) {
        inv_item->current_frame = 0;
    } else if (inv_item->current_frame < 0) {
        inv_item->current_frame = inv_item->frames_total - 1;
    }
    InvRing_SelectMeshes(inv_item);
    return true;
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
