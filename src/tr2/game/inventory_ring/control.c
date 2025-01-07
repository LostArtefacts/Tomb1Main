#include "game/inventory_ring/control.h"

#include "decomp/savegame.h"
#include "game/clock.h"
#include "game/demo.h"
#include "game/game.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/inventory.h"
#include "game/inventory_ring/draw.h"
#include "game/inventory_ring/vars.h"
#include "game/lara/control.h"
#include "game/matrix.h"
#include "game/music.h"
#include "game/option/option.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "game/viewport.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/inventory_ring/priv.h>
#include <libtrx/game/objects/names.h>
#include <libtrx/game/objects/vars.h>
#include <libtrx/memory.h>

#include <stdio.h>

#define TITLE_RING_OBJECTS 3
#define OPTION_RING_OBJECTS 3

static int32_t m_NoInputCounter = 0;

static void M_ShowAmmoQuantity(const char *fmt, int32_t qty);

static void M_RingIsOpen(INV_RING *ring);
static void M_RingIsNotOpen(INV_RING *ring);
static void M_RingNotActive(const INVENTORY_ITEM *inv_item);
static void M_RingActive(void);

static bool M_AnimateInventoryItem(INVENTORY_ITEM *inv_item);

static GAME_FLOW_COMMAND M_Control(INV_RING *ring);

static void M_ShowAmmoQuantity(const char *const fmt, const int32_t qty)
{
    if (!g_SaveGame.bonus_flag) {
        InvRing_ShowItemQuantity(fmt, qty);
    }
}

static void M_RingIsOpen(INV_RING *const ring)
{
    InvRing_ShowHeader(ring);
}

static void M_RingIsNotOpen(INV_RING *const ring)
{
    InvRing_RemoveHeader();
}

static void M_RingNotActive(const INVENTORY_ITEM *const inv_item)
{
    InvRing_ShowItemName(inv_item);

    const int32_t qty = Inv_RequestItem(inv_item->object_id);
    switch (inv_item->object_id) {
    case O_SHOTGUN_OPTION:
        M_ShowAmmoQuantity("%5d", g_Lara.shotgun_ammo.ammo / SHOTGUN_AMMO_CLIP);
        break;
    case O_MAGNUM_OPTION:
        M_ShowAmmoQuantity("%5d", g_Lara.magnum_ammo.ammo);
        break;
    case O_UZI_OPTION:
        M_ShowAmmoQuantity("%5d", g_Lara.uzi_ammo.ammo);
        break;
    case O_HARPOON_OPTION:
        M_ShowAmmoQuantity("%5d", g_Lara.harpoon_ammo.ammo);
        break;
    case O_M16_OPTION:
        M_ShowAmmoQuantity("%5d", g_Lara.m16_ammo.ammo);
        break;
    case O_GRENADE_OPTION:
        M_ShowAmmoQuantity("%5d", g_Lara.grenade_ammo.ammo);
        break;
    case O_SHOTGUN_AMMO_OPTION:
        M_ShowAmmoQuantity("%d", SHOTGUN_SHELL_COUNT * qty);
        break;

    case O_MAGNUM_AMMO_OPTION:
    case O_UZI_AMMO_OPTION:
    case O_HARPOON_AMMO_OPTION:
    case O_M16_AMMO_OPTION:
        InvRing_ShowItemQuantity("%d", 2 * qty);
        break;

    case O_GRENADE_AMMO_OPTION:
    case O_FLARES_OPTION:
        InvRing_ShowItemQuantity("%d", qty);
        break;

    case O_SMALL_MEDIPACK_OPTION:
    case O_LARGE_MEDIPACK_OPTION:
        g_HealthBarTimer = 40;
        Overlay_DrawHealthBar();
        InvRing_ShowItemQuantity("%d", qty);
        break;

    case O_PUZZLE_OPTION_1:
    case O_PUZZLE_OPTION_2:
    case O_PUZZLE_OPTION_3:
    case O_PUZZLE_OPTION_4:
    case O_KEY_OPTION_1:
    case O_KEY_OPTION_2:
    case O_KEY_OPTION_3:
    case O_KEY_OPTION_4:
    case O_PICKUP_OPTION_1:
    case O_PICKUP_OPTION_2:
        if (qty > 1) {
            InvRing_ShowItemQuantity("%d", qty);
        }
        break;

    default:
        break;
    }

    InvRing_HideArrow(
        INV_RING_ARROW_TL,
        inv_item->object_id == O_SMALL_MEDIPACK_OPTION
            || inv_item->object_id == O_LARGE_MEDIPACK_OPTION);
}

static void M_RingActive(void)
{
    InvRing_RemoveItemTexts();
}

static bool M_AnimateInventoryItem(INVENTORY_ITEM *const inv_item)
{
    if (inv_item->current_frame == inv_item->goal_frame) {
        InvRing_SelectMeshes(inv_item);
        return false;
    }

    if (inv_item->anim_count > 0) {
        inv_item->anim_count--;
    } else {
        inv_item->anim_count = inv_item->anim_speed;
        inv_item->current_frame += inv_item->anim_direction;
        if (inv_item->current_frame >= inv_item->frames_total) {
            inv_item->current_frame = 0;
        } else if (inv_item->current_frame < 0) {
            inv_item->current_frame = inv_item->frames_total - 1;
        }
    }

    InvRing_SelectMeshes(inv_item);
    return true;
}

static GAME_FLOW_COMMAND M_Control(INV_RING *const ring)
{
    if (g_GF_OverrideCommand.action != GF_NOOP) {
        return g_GF_OverrideCommand;
    }

    if (!ring->has_spun_out) {
        Sound_Effect(SFX_MENU_SPININ, NULL, SPM_ALWAYS);
        ring->has_spun_out = true;
    }

    InvRing_CalcAdders(ring, INV_RING_ROTATE_DURATION);
    Shell_ProcessEvents();
    Input_Update();
    Shell_ProcessInput();
    Game_ProcessInput();

    if (g_Inv_DemoMode) {
        if (g_InputDB.any) {
            return GF_TranslateScriptCommand(g_GameFlow.on_demo_interrupt);
        }
        if (!Demo_GetInput()) {
            return GF_TranslateScriptCommand(g_GameFlow.on_demo_end);
        }
    } else if (ring->mode != INV_TITLE_MODE || g_Input.any || g_InputDB.any) {
        m_NoInputCounter = 0;
    } else if (g_GameFlow.num_demos > 0 && ring->motion.status == RNG_OPEN) {
        m_NoInputCounter++;
        if (m_NoInputCounter > g_GameFlow.no_input_time) {
            ring->is_demo_needed = true;
        }
    }

    if (g_IsGameToExit) {
        return (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
    }

    if ((ring->mode == INV_SAVE_MODE || ring->mode == INV_LOAD_MODE
         || ring->mode == INV_DEATH_MODE)
        && !ring->is_pass_open) {
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0, .menu_confirm = 1 };
    }

    for (int32_t frame = 0; frame < INV_RING_FRAMES; frame++) {
        if (g_Inv_IsOptionsDelay) {
            if (g_Inv_OptionsDelayCounter) {
                g_Inv_OptionsDelayCounter--;
            } else {
                g_Inv_IsOptionsDelay = false;
            }
        }
        InvRing_DoMotions(ring);
    }

    if (!ring->rotating) {
        switch (ring->motion.status) {
        case RNG_OPEN:
            if (g_Input.right && ring->number_of_objects > 1) {
                InvRing_RotateLeft(ring);
                Sound_Effect(SFX_MENU_ROTATE, 0, SPM_ALWAYS);
                break;
            }

            if (g_Input.left && ring->number_of_objects > 1) {
                InvRing_RotateRight(ring);
                Sound_Effect(SFX_MENU_ROTATE, 0, SPM_ALWAYS);
                break;
            }

            if (ring->is_demo_needed
                || ((g_InputDB.option || g_InputDB.menu_back)
                    && ring->mode != INV_TITLE_MODE)) {
                Sound_Effect(SFX_MENU_SPINOUT, 0, SPM_ALWAYS);
                g_Inv_Chosen = NO_OBJECT;
                if (ring->type != RT_MAIN) {
                    g_InvRing_Source[RT_OPTION].current = ring->current_object;
                } else {
                    g_InvRing_Source[RT_MAIN].current = ring->current_object;
                }
                InvRing_MotionSetup(ring, RNG_CLOSING, RNG_DONE, 32);
                InvRing_MotionRadius(ring, 0);
                InvRing_MotionCameraPos(ring, INV_RING_CAMERA_START_HEIGHT);
                InvRing_MotionRotation(
                    ring, PHD_180, ring->ring_pos.rot.y + PHD_180);
                g_Input = (INPUT_STATE) { 0 };
                g_InputDB = (INPUT_STATE) { 0 };
            }

            if (g_InputDB.menu_confirm) {
                if ((ring->mode == INV_SAVE_MODE || ring->mode == INV_LOAD_MODE
                     || ring->mode == INV_DEATH_MODE)
                    && !ring->is_pass_open) {
                    ring->is_pass_open = true;
                }

                g_SoundOptionLine = 0;
                INVENTORY_ITEM *inv_item;
                if (ring->type == RT_MAIN) {
                    g_InvRing_Source[RT_MAIN].current = ring->current_object;
                    inv_item =
                        g_InvRing_Source[RT_MAIN].items[ring->current_object];
                } else if (ring->type == RT_OPTION) {
                    g_InvRing_Source[RT_OPTION].current = ring->current_object;
                    inv_item =
                        g_InvRing_Source[RT_OPTION].items[ring->current_object];
                } else {
                    g_InvRing_Source[RT_KEYS].current = ring->current_object;
                    inv_item =
                        g_InvRing_Source[RT_KEYS].items[ring->current_object];
                }

                inv_item->goal_frame = inv_item->open_frame;
                inv_item->anim_direction = 1;
                InvRing_MotionSetup(ring, RNG_SELECTING, RNG_SELECTED, 16);
                InvRing_MotionRotation(
                    ring, 0,
                    -DEG_90 - ring->angle_adder * ring->current_object);
                InvRing_MotionItemSelect(ring, inv_item);
                g_Input = (INPUT_STATE) { 0 };
                g_InputDB = (INPUT_STATE) { 0 };

                switch (inv_item->object_id) {
                case O_COMPASS_OPTION:
                    Sound_Effect(SFX_MENU_STOPWATCH, 0, SPM_ALWAYS);
                    break;

                case O_PHOTO_OPTION:
                    Sound_Effect(SFX_MENU_LARA_HOME, 0, SPM_ALWAYS);
                    break;

                case O_PISTOL_OPTION:
                case O_SHOTGUN_OPTION:
                case O_MAGNUM_OPTION:
                case O_UZI_OPTION:
                case O_HARPOON_OPTION:
                case O_M16_OPTION:
                case O_GRENADE_OPTION:
                    Sound_Effect(SFX_MENU_GUNS, 0, SPM_ALWAYS);
                    break;

                default:
                    Sound_Effect(SFX_MENU_SPININ, 0, SPM_ALWAYS);
                    break;
                }
            }

            if (g_InputDB.forward && ring->mode != INV_TITLE_MODE
                && ring->mode != INV_KEYS_MODE) {
                if (ring->type == RT_OPTION) {
                    if (g_InvRing_Source[RT_MAIN].count > 0) {
                        InvRing_MotionSetup(
                            ring, RNG_CLOSING, RNG_OPTION2MAIN, 24);
                        InvRing_MotionRadius(ring, 0);
                        InvRing_MotionRotation(
                            ring, PHD_180, ring->ring_pos.rot.y + PHD_180);
                        InvRing_MotionCameraPitch(ring, 0x2000);
                        ring->motion.misc = 0x2000;
                    }
                    g_InputDB = (INPUT_STATE) { 0 };
                } else if (ring->type == RT_MAIN) {
                    if (g_InvRing_Source[RT_KEYS].count > 0) {
                        InvRing_MotionSetup(
                            ring, RNG_CLOSING, RNG_MAIN2KEYS, 24);
                        InvRing_MotionRadius(ring, 0);
                        InvRing_MotionRotation(
                            ring, PHD_180, ring->ring_pos.rot.y + PHD_180);
                        InvRing_MotionCameraPitch(ring, 0x2000);
                        ring->motion.misc = 0x2000;
                    }
                    g_Input = (INPUT_STATE) { 0 };
                    g_InputDB = (INPUT_STATE) { 0 };
                }
            } else if (
                g_InputDB.back && ring->mode != INV_TITLE_MODE
                && ring->mode != INV_KEYS_MODE) {
                if (ring->type == RT_KEYS) {
                    if (g_InvRing_Source[RT_MAIN].count > 0) {
                        InvRing_MotionSetup(
                            ring, RNG_CLOSING, RNG_KEYS2MAIN, 24);
                        InvRing_MotionRadius(ring, 0);
                        InvRing_MotionRotation(
                            ring, PHD_180, ring->ring_pos.rot.y + PHD_180);
                        InvRing_MotionCameraPitch(ring, -0x2000);
                        ring->motion.misc = -0x2000;
                    }
                    g_Input = (INPUT_STATE) { 0 };
                    g_InputDB = (INPUT_STATE) { 0 };
                } else if (ring->type == RT_MAIN) {
                    if (g_InvRing_Source[RT_OPTION].count > 0
                        && !g_GameFlow.lockout_option_ring) {
                        InvRing_MotionSetup(
                            ring, RNG_CLOSING, RNG_MAIN2OPTION, 24);
                        InvRing_MotionRadius(ring, 0);
                        InvRing_MotionRotation(
                            ring, PHD_180, ring->ring_pos.rot.y + PHD_180);
                        InvRing_MotionCameraPitch(ring, -0x2000);
                        ring->motion.misc = -0x2000;
                    }
                    g_InputDB = (INPUT_STATE) { 0 };
                }
            }
            break;

        case RNG_MAIN2OPTION:
            InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, 24);
            InvRing_MotionRadius(ring, INV_RING_RADIUS);
            ring->camera_pitch = -(int16_t)(ring->motion.misc);
            ring->motion.camera_pitch_rate = ring->motion.misc / 24;
            ring->motion.camera_pitch_target = 0;
            ring->list = g_InvRing_Source[RT_OPTION].items;
            ring->type = RT_OPTION;
            g_InvRing_Source[RT_MAIN].current = ring->current_object;
            g_InvRing_Source[RT_MAIN].count = ring->number_of_objects;
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
            InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, 24);
            InvRing_MotionRadius(ring, INV_RING_RADIUS);
            ring->motion.camera_pitch_target = 0;
            ring->camera_pitch = -(int16_t)(ring->motion.misc);
            ring->motion.camera_pitch_rate = ring->motion.misc / 24;
            g_InvRing_Source[RT_MAIN].current = ring->current_object;
            g_InvRing_Source[RT_MAIN].count = ring->number_of_objects;
            ring->list = g_InvRing_Source[RT_KEYS].items;
            ring->type = RT_KEYS;
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
            InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, 24);
            InvRing_MotionRadius(ring, INV_RING_RADIUS);
            ring->camera_pitch = -(int16_t)(ring->motion.misc);
            ring->motion.camera_pitch_rate = ring->motion.misc / 24;
            ring->motion.camera_pitch_target = 0;
            ring->list = g_InvRing_Source[RT_MAIN].items;
            ring->type = RT_MAIN;
            g_InvRing_Source[RT_KEYS].current = ring->current_object;
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
            InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, 24);
            InvRing_MotionRadius(ring, INV_RING_RADIUS);
            ring->camera_pitch = -(int16_t)(ring->motion.misc);
            ring->motion.camera_pitch_rate = ring->motion.misc / 24;
            g_InvRing_Source[RT_OPTION].current = ring->current_object;
            g_InvRing_Source[RT_OPTION].count = ring->number_of_objects;
            ring->motion.camera_pitch_target = 0;
            ring->list = g_InvRing_Source[RT_MAIN].items;
            ring->type = RT_MAIN;
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
            for (int32_t frame = 0; frame < INV_RING_FRAMES; frame++) {
                busy = false;
                if (inv_item->y_rot == inv_item->y_rot_sel) {
                    busy = M_AnimateInventoryItem(inv_item);
                }
            }

            if (!busy && !g_Inv_IsOptionsDelay) {
                Option_Control(inv_item);

                if (g_InputDB.menu_back) {
                    inv_item->sprite_list = NULL;
                    InvRing_MotionSetup(
                        ring, RNG_CLOSING_ITEM, RNG_DESELECT, 0);
                    g_Input = (INPUT_STATE) { 0 };
                    g_InputDB = (INPUT_STATE) { 0 };
                    if (ring->mode == INV_LOAD_MODE
                        || ring->mode == INV_SAVE_MODE) {
                        InvRing_MotionSetup(
                            ring, RNG_CLOSING_ITEM, RNG_EXITING_INVENTORY, 0);
                        g_Input = (INPUT_STATE) { 0 };
                        g_InputDB = (INPUT_STATE) { 0 };
                    }
                }

                if (g_InputDB.menu_confirm) {
                    inv_item->sprite_list = NULL;
                    g_Inv_Chosen = inv_item->object_id;
                    if (ring->type != RT_MAIN) {
                        g_InvRing_Source[RT_OPTION].current =
                            ring->current_object;
                    } else {
                        g_InvRing_Source[RT_MAIN].current =
                            ring->current_object;
                    }
                    if (ring->mode == INV_TITLE_MODE
                        && (inv_item->object_id == O_DETAIL_OPTION
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
            Sound_Effect(SFX_MENU_SPINOUT, 0, SPM_ALWAYS);
            InvRing_MotionSetup(ring, RNG_DESELECTING, RNG_OPEN, 16);
            InvRing_MotionRotation(
                ring, 0, -DEG_90 - ring->angle_adder * ring->current_object);
            g_Input = (INPUT_STATE) { 0 };
            g_InputDB = (INPUT_STATE) { 0 };
            break;

        case RNG_CLOSING_ITEM: {
            INVENTORY_ITEM *inv_item = ring->list[ring->current_object];
            for (int32_t frame = 0; frame < INV_RING_FRAMES; frame++) {
                if (!M_AnimateInventoryItem(inv_item)) {
                    if (inv_item->object_id == O_PASSPORT_OPTION) {
                        inv_item->object_id = O_PASSPORT_CLOSED;
                        inv_item->current_frame = 0;
                    }

                    ring->motion.count = 16;
                    ring->motion.status = ring->motion.status_target;
                    InvRing_MotionItemDeselect(ring, inv_item);
                    break;
                }
            }
            break;
        }

        case RNG_EXITING_INVENTORY:
            if (ring->motion.count == 0) {
                InvRing_MotionSetup(ring, RNG_CLOSING, RNG_DONE, 32);
                InvRing_MotionRadius(ring, 0);
                InvRing_MotionCameraPos(ring, INV_RING_CAMERA_START_HEIGHT);
                InvRing_MotionRotation(
                    ring, PHD_180, ring->ring_pos.rot.y + PHD_180);
            }
            break;

        default:
            break;
        }
    }

    if (ring->motion.status == RNG_OPEN || ring->motion.status == RNG_SELECTING
        || ring->motion.status == RNG_SELECTED
        || ring->motion.status == RNG_DESELECTING
        || ring->motion.status == RNG_DESELECT
        || ring->motion.status == RNG_CLOSING_ITEM) {
        if (!ring->rotating && !g_Input.menu_left && !g_Input.menu_right) {
            INVENTORY_ITEM *const inv_item = ring->list[ring->current_object];
            M_RingNotActive(inv_item);
        }
        M_RingIsOpen(ring);
    } else {
        M_RingIsNotOpen(ring);
    }

    if (ring->motion.status == RNG_OPENING || ring->motion.status == RNG_CLOSING
        || ring->motion.status == RNG_MAIN2OPTION
        || ring->motion.status == RNG_OPTION2MAIN
        || ring->motion.status == RNG_EXITING_INVENTORY
        || ring->motion.status == RNG_DONE || ring->rotating) {
        M_RingActive();
    }

    if (g_CurrentLevel != LV_GYM) {
        Stats_UpdateTimer();
    }

    for (int32_t i = 0; i < ring->number_of_objects; i++) {
        InvRing_UpdateInventoryItem(ring, ring->list[i], INV_RING_FRAMES);
    }

    Sound_EndScene();
    return (GAME_FLOW_COMMAND) { .action = GF_NOOP };
}

void InvRing_RemoveAllText(void)
{
    InvRing_RemoveHeader();
    InvRing_RemoveItemTexts();
}

INV_RING *InvRing_Open(const INVENTORY_MODE mode)
{
    if (mode == INV_KEYS_MODE && g_InvRing_Source[RT_KEYS].count == 0) {
        g_Inv_Chosen = NO_OBJECT;
        return NULL;
    }

    Clock_SyncTick();

    g_PhdWinRight = g_PhdWinMaxX;
    g_PhdWinLeft = 0;
    g_PhdWinTop = 0;
    g_PhdWinBottom = g_PhdWinMaxY;
    g_Inv_Chosen = NO_OBJECT;

    if (mode == INV_TITLE_MODE) {
        g_InvRing_Source[RT_OPTION].count = TITLE_RING_OBJECTS;
        if (g_GameFlow.gym_enabled) {
            g_InvRing_Source[RT_OPTION].count++;
        }
        InvRing_ShowVersionText();
    } else {
        g_InvRing_Source[RT_OPTION].count = OPTION_RING_OBJECTS;
        InvRing_RemoveVersionText();
    }

    for (int32_t i = 0; i < 8; i++) {
        g_Inv_ExtraData[i] = 0;
    }

    g_InvRing_Source[RT_MAIN].current = 0;
    for (int32_t i = 0; i < g_InvRing_Source[RT_MAIN].count; i++) {
        InvRing_InitInvItem(g_InvRing_Source[RT_MAIN].items[i]);
    }
    for (int32_t i = 0; i < g_InvRing_Source[RT_OPTION].count; i++) {
        InvRing_InitInvItem(g_InvRing_Source[RT_OPTION].items[i]);
    }

    g_InvRing_Source[RT_OPTION].current = 0;
    if (g_GymInvOpenEnabled && mode == INV_TITLE_MODE
        && !g_GameFlow.load_save_disabled && g_GameFlow.gym_enabled) {
        for (int32_t i = 0; i < g_InvRing_Source[RT_OPTION].count; i++) {
            if (g_InvRing_Source[RT_OPTION].items[i]->object_id
                == O_PHOTO_OPTION) {
                g_InvRing_Source[RT_OPTION].current = i;
            }
        }
    }

    g_SoundOptionLine = 0;

    INV_RING *const ring = Memory_Alloc(sizeof(INV_RING));
    ring->mode = mode;
    ring->old_fov = Viewport_GetFOV();
    m_NoInputCounter = 0;

    switch (mode) {
    case INV_TITLE_MODE:
    case INV_SAVE_MODE:
    case INV_LOAD_MODE:
    case INV_DEATH_MODE:
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
        if (g_InvRing_Source[RT_MAIN].count > 0) {
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

    g_Inv_Mode = mode;

    if (g_CurrentLevel != LV_GYM) {
        Stats_StartTimer();
    }

    if (mode == INV_TITLE_MODE) {
        Output_LoadBackgroundFromFile("data/title.pcx");
    } else {
        Output_LoadBackgroundFromObject();
    }
    Overlay_HideGameInfo();

    Sound_StopAllSamples();
    if (mode != INV_TITLE_MODE) {
        Music_Pause();
    }
    Viewport_AlterFOV(80 * PHD_DEGREE);

    return ring;
}

GAME_FLOW_COMMAND InvRing_Close(INV_RING *const ring)
{
    GAME_FLOW_COMMAND gf_cmd = { .action = GF_NOOP };

    InvRing_RemoveAllText();
    InvRing_RemoveVersionText();

    if (ring->list != NULL) {
        INVENTORY_ITEM *const inv_item = ring->list[ring->current_object];
        if (inv_item != NULL) {
            Option_Shutdown(inv_item);
        }
    }
    if (ring->mode == INV_TITLE_MODE) {
        Music_Stop();
    }
    Viewport_AlterFOV(ring->old_fov);
    Output_UnloadBackground();

    // enable buffering
    g_OldInputDB = (INPUT_STATE) { 0 };

    if (g_IsGameToExit) {
        gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
        goto finish;
    } else if (g_GF_OverrideCommand.action != GF_NOOP) {
        gf_cmd = g_GF_OverrideCommand;
        goto finish;
    } else if (ring->is_demo_needed) {
        gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_START_DEMO, .param = -1 };
        goto finish;
    } else if (g_Inv_Chosen == NO_OBJECT) {
        if (ring->mode != INV_TITLE_MODE) {
            Music_Unpause();
        }
    } else {
        if (ring->mode != INV_TITLE_MODE) {
            Music_Unpause();
        }
        if (g_Config.gameplay.fix_item_duplication_glitch) {
            Inv_ClearSelection();
        }

        switch (g_Inv_Chosen) {
        case O_PASSPORT_OPTION:
            if (g_Inv_ExtraData[0] == 1) {
                Music_Unpause();
            }

            if (g_Inv_ExtraData[0] == 0) {
                // first passport page: load game.
                Inv_RemoveAllItems();
                S_LoadGame(
                    &g_SaveGame, sizeof(SAVEGAME_INFO), g_Inv_ExtraData[1]);
                gf_cmd = (GAME_FLOW_COMMAND) {
                    .action = GF_START_SAVED_GAME,
                    .param = g_Inv_ExtraData[1],
                };
                goto finish;
            } else if (g_Inv_ExtraData[0] == 1) {
                // second passport page:
                if (ring->mode == INV_TITLE_MODE) {
                    // title mode - new game or select level.
                    if (g_GameFlow.play_any_level) {
                        gf_cmd = (GAME_FLOW_COMMAND) {
                            .action = GF_START_GAME,
                            .param = g_Inv_ExtraData[1] + 1,
                        };
                        goto finish;
                    } else {
                        InitialiseStartInfo();
                        gf_cmd = (GAME_FLOW_COMMAND) {
                            .action = GF_START_GAME,
                            .param = LV_FIRST,
                        };
                        goto finish;
                    }
                } else {
                    // game mode - save game (or start the game if in Lara's
                    // Home)
                    if (g_CurrentLevel == LV_GYM) {
                        InitialiseStartInfo();
                        gf_cmd = (GAME_FLOW_COMMAND) {
                            .action = GF_START_GAME,
                            .param = LV_FIRST,
                        };
                        goto finish;
                    } else {
                        CreateSaveGameInfo();
                        const int16_t slot_num = g_Inv_ExtraData[1];
                        S_SaveGame(
                            &g_SaveGame, sizeof(SAVEGAME_INFO), slot_num);
                    }
                }
            } else {
                // third passport page:
                if (ring->mode == INV_TITLE_MODE) {
                    // title mode - exit the game
                    gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_GAME };
                    goto finish;
                } else {
                    // game mode - exit to title
                    gf_cmd = (GAME_FLOW_COMMAND) { .action = GF_EXIT_TO_TITLE };
                    goto finish;
                }
            }
            break;

        case O_PHOTO_OPTION:
            if (g_GameFlow.gym_enabled) {
                gf_cmd = (GAME_FLOW_COMMAND) {
                    .action = GF_START_GAME,
                    .param = LV_GYM,
                };
                goto finish;
            }
            break;

        case O_PISTOL_OPTION:
        case O_SHOTGUN_OPTION:
        case O_MAGNUM_OPTION:
        case O_UZI_OPTION:
        case O_HARPOON_OPTION:
        case O_M16_OPTION:
        case O_GRENADE_OPTION:
        case O_SMALL_MEDIPACK_OPTION:
        case O_LARGE_MEDIPACK_OPTION:
        case O_FLARES_OPTION:
            Lara_UseItem(g_Inv_Chosen);
            break;

        default:
            break;
        }
    }

finish:
    Memory_Free(ring);
    return gf_cmd;
}

GAME_FLOW_COMMAND InvRing_Control(
    INV_RING *const ring, const int32_t num_frames)
{
    GAME_FLOW_COMMAND gf_cmd = { .action = GF_NOOP };
    for (int32_t i = 0; i < num_frames; i++) {
        gf_cmd = M_Control(ring);
        if (gf_cmd.action != GF_NOOP) {
            break;
        }
    }

    g_Camera.num_frames = num_frames * TICKS_PER_FRAME;
    Overlay_Animate(num_frames);
    Output_AnimateTextures(g_Camera.num_frames);
    return gf_cmd;
}

bool InvRing_IsOptionLockedOut(void)
{
    return g_GameFlow.lockout_option_ring;
}
