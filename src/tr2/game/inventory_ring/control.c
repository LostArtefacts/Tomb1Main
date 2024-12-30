#include "game/inventory_ring/control.h"

#include "config.h"
#include "decomp/savegame.h"
#include "game/clock.h"
#include "game/demo.h"
#include "game/game.h"
#include "game/input.h"
#include "game/inventory.h"
#include "game/inventory_ring/draw.h"
#include "game/inventory_ring/priv.h"
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
#include "global/vars.h"

#include <libtrx/game/objects/names.h>
#include <libtrx/game/objects/vars.h>
#include <libtrx/memory.h>

#include <stdio.h>

#define TITLE_RING_OBJECTS 3
#define OPTION_RING_OBJECTS 3

static TEXTSTRING *m_UpArrow1 = NULL;
static TEXTSTRING *m_UpArrow2 = NULL;
static TEXTSTRING *m_DownArrow1 = NULL;
static TEXTSTRING *m_DownArrow2 = NULL;
static TEXTSTRING *m_VersionText = NULL;
static int32_t m_NoInputCounter = 0;

static void M_RemoveItemsText(void);
static void M_RemoveAllText(void);
static void M_ShowItemQuantity(const char *fmt, int32_t qty);
static void M_ShowAmmoQuantity(const char *fmt, int32_t qty);
static void M_End(INV_RING *ring);

static void M_Construct(INVENTORY_MODE mode);
static void M_RingIsOpen(INV_RING *ring);
static void M_RingIsNotOpen(INV_RING *ring);
static void M_RingNotActive(const INV_ITEM *inv_item);
static void M_RingActive(void);
static void M_SelectMeshes(INV_ITEM *inv_item);
static void M_UpdateInventoryItem(
    const INV_RING *ring, INV_ITEM *inv_item, int32_t num_frames);
static bool M_AnimateInventoryItem(INV_ITEM *inv_item);

static void M_RemoveItemsText(void)
{
    for (int32_t i = 0; i < 2; i++) {
        Text_Remove(g_Inv_ItemText[i]);
        g_Inv_ItemText[i] = NULL;
    }
}

static void M_RemoveAllText(void)
{
    M_RemoveItemsText();

    Text_Remove(g_Inv_TagText);
    g_Inv_TagText = NULL;
    Text_Remove(g_Inv_RingText);
    g_Inv_RingText = NULL;
    Text_Remove(m_UpArrow1);
    m_UpArrow1 = NULL;
    Text_Remove(m_UpArrow2);
    m_UpArrow2 = NULL;
    Text_Remove(m_DownArrow1);
    m_DownArrow1 = NULL;
    Text_Remove(m_DownArrow2);
    m_DownArrow2 = NULL;

    Text_Remove(m_VersionText);
    m_VersionText = NULL;
}

static void M_ShowItemQuantity(const char *const fmt, const int32_t qty)
{
    if (g_Inv_ItemText[1] == NULL && !g_SaveGame.bonus_flag) {
        char string[64];
        sprintf(string, fmt, qty);
        Overlay_MakeAmmoString(string);
        g_Inv_ItemText[1] = Text_Create(64, -56, string);
        Text_AlignBottom(g_Inv_ItemText[1], true);
        Text_CentreH(g_Inv_ItemText[1], true);
    }
}

static void M_ShowAmmoQuantity(const char *const fmt, const int32_t qty)
{
    if (!g_SaveGame.bonus_flag) {
        M_ShowItemQuantity(fmt, qty);
    }
}

static void M_Construct(const INVENTORY_MODE mode)
{
    if (mode == INV_TITLE_MODE) {
        g_Inv_OptionObjectsCount = TITLE_RING_OBJECTS;
        if (g_GameFlow.gym_enabled) {
            g_Inv_OptionObjectsCount++;
        }
        m_VersionText = Text_Create(-20, -18, g_TR2XVersion);
        Text_AlignRight(m_VersionText, 1);
        Text_AlignBottom(m_VersionText, 1);
        Text_SetScale(
            m_VersionText, TEXT_BASE_SCALE * 0.5, TEXT_BASE_SCALE * 0.5);
    } else {
        g_Inv_OptionObjectsCount = OPTION_RING_OBJECTS;
        Text_Remove(m_VersionText);
        m_VersionText = NULL;
    }

    for (int32_t i = 0; i < 8; i++) {
        g_Inv_ExtraData[i] = 0;
    }

    for (int32_t i = 0; i < g_Inv_MainObjectsCount; i++) {
        INV_ITEM *const inv_item = g_Inv_MainList[i];
        inv_item->meshes_drawn = inv_item->meshes_sel;
        inv_item->current_frame = 0;
        inv_item->goal_frame = 0;
        inv_item->anim_count = 0;
        inv_item->x_rot_pt = 0;
        inv_item->x_rot = 0;
        inv_item->y_rot = 0;
        inv_item->y_trans = 0;
        inv_item->z_trans = 0;
        if (inv_item->object_id == O_PASSPORT_OPTION) {
            inv_item->object_id = O_PASSPORT_CLOSED;
        }
    }

    for (int32_t i = 0; i < g_Inv_OptionObjectsCount; i++) {
        INV_ITEM *const inv_item = g_Inv_OptionList[i];
        inv_item->current_frame = 0;
        inv_item->goal_frame = 0;
        inv_item->anim_count = 0;
        inv_item->x_rot_pt = 0;
        inv_item->x_rot = 0;
        inv_item->y_rot = 0;
        inv_item->y_trans = 0;
        inv_item->z_trans = 0;
        if (inv_item->object_id == O_PASSPORT_OPTION) {
            inv_item->object_id = O_PASSPORT_CLOSED;
        }
    }

    g_Inv_MainCurrent = 0;

    if (g_GymInvOpenEnabled && mode == INV_TITLE_MODE
        && !g_GameFlow.load_save_disabled && g_GameFlow.gym_enabled) {
        g_Inv_OptionCurrent = 3; // TODO: don't hardcode me
    } else {
        g_Inv_OptionCurrent = 0;
    }
    g_SoundOptionLine = 0;
}

static void M_End(INV_RING *const ring)
{
    M_RemoveAllText();
    if (ring->list != NULL) {
        INV_ITEM *const inv_item = ring->list[ring->current_object];
        if (inv_item != NULL) {
            Option_Shutdown(inv_item);
        }
    }
    if (ring->mode == INV_TITLE_MODE) {
        Music_Stop();
    }
    Memory_Free(ring);
    Output_UnloadBackground();
}

static void M_RingIsOpen(INV_RING *const ring)
{
    if (ring->mode == INV_TITLE_MODE) {
        return;
    }

    if (g_Inv_RingText == NULL) {
        switch (ring->type) {
        case RT_MAIN:
            g_Inv_RingText = Text_Create(
                0, 26, g_GF_GameStrings[GF_S_GAME_HEADING_INVENTORY]);
            break;

        case RT_OPTION:
            if (ring->mode == INV_DEATH_MODE) {
                g_Inv_RingText = Text_Create(
                    0, 26, g_GF_GameStrings[GF_S_GAME_HEADING_GAME_OVER]);
            } else {
                g_Inv_RingText = Text_Create(
                    0, 26, g_GF_GameStrings[GF_S_GAME_HEADING_OPTION]);
            }
            Text_CentreH(g_Inv_RingText, true);
            break;

        case RT_KEYS:
            g_Inv_RingText =
                Text_Create(0, 26, g_GF_GameStrings[GF_S_GAME_HEADING_ITEMS]);
            break;
        }

        Text_CentreH(g_Inv_RingText, true);
    }

    if (ring->mode == INV_KEYS_MODE || ring->mode == INV_DEATH_MODE) {
        return;
    }

    if (m_UpArrow1 == NULL) {
        if (ring->type == RT_OPTION
            || (ring->type == RT_MAIN && g_Inv_KeyObjectsCount > 0)) {
            m_UpArrow1 = Text_Create(20, 28, "\\{arrow up}");
            m_UpArrow2 = Text_Create(-20, 28, "\\{arrow up}");
            Text_AlignRight(m_UpArrow2, true);
        }
    }

    if (m_DownArrow1 == NULL
        && ((
            (ring->type == RT_MAIN && !g_GameFlow.lockout_option_ring)
            || ring->type == RT_KEYS))) {
        m_DownArrow1 = Text_Create(20, -15, "\\{arrow down}");
        Text_AlignBottom(m_DownArrow1, true);
        m_DownArrow2 = Text_Create(-20, -15, "\\{arrow down}");
        Text_AlignBottom(m_DownArrow2, true);
        Text_AlignRight(m_DownArrow2, true);
    }
}

static void M_RingIsNotOpen(INV_RING *const ring)
{
    Text_Remove(g_Inv_TagText);
    g_Inv_TagText = NULL;
    Text_Remove(g_Inv_RingText);
    g_Inv_RingText = NULL;
    Text_Remove(m_UpArrow1);
    m_UpArrow1 = NULL;
    Text_Remove(m_UpArrow2);
    m_UpArrow2 = NULL;
    Text_Remove(m_DownArrow1);
    m_DownArrow1 = NULL;
    Text_Remove(m_DownArrow2);
    m_DownArrow2 = NULL;
}

static void M_RingNotActive(const INV_ITEM *const inv_item)
{
    if (g_Inv_ItemText[0] == NULL) {
        if (inv_item->object_id != O_PASSPORT_OPTION) {
            g_Inv_ItemText[0] =
                Text_Create(0, -16, Object_GetName(inv_item->object_id));
        }

        if (g_Inv_ItemText[0]) {
            Text_AlignBottom(g_Inv_ItemText[0], true);
            Text_CentreH(g_Inv_ItemText[0], true);
        }
    }

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
        M_ShowAmmoQuantity("%d", 2 * qty);
        break;

    case O_GRENADE_AMMO_OPTION:
    case O_FLARES_OPTION:
        M_ShowAmmoQuantity("%d", qty);
        break;

    case O_SMALL_MEDIPACK_OPTION:
    case O_LARGE_MEDIPACK_OPTION:
        g_HealthBarTimer = 40;
        Overlay_DrawHealthBar();
        M_ShowItemQuantity("%d", qty);
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
            M_ShowItemQuantity("%d", qty);
        }
        break;

    default:
        break;
    }
}

static void M_RingActive(void)
{
    M_RemoveItemsText();
}

static void M_SelectMeshes(INV_ITEM *const inv_item)
{
    switch (inv_item->object_id) {
    case O_PASSPORT_OPTION:
        if (inv_item->current_frame < 4) {
            inv_item->meshes_drawn = PM_COMMON | PM_IN_FRONT;
        } else if (inv_item->current_frame <= 16) {
            inv_item->meshes_drawn = PM_COMMON | PM_IN_FRONT | PM_PAGE_1;
        } else if (inv_item->current_frame < 19) {
            inv_item->meshes_drawn =
                PM_COMMON | PM_IN_FRONT | PM_PAGE_1 | PM_PAGE_2;
        } else if (inv_item->current_frame == 19) {
            inv_item->meshes_drawn = PM_COMMON | PM_PAGE_1 | PM_PAGE_2;
        } else if (inv_item->current_frame < 24) {
            inv_item->meshes_drawn =
                PM_COMMON | PM_IN_BACK | PM_PAGE_1 | PM_PAGE_2;
        } else if (inv_item->current_frame < 29) {
            inv_item->meshes_drawn = PM_COMMON | PM_IN_BACK | PM_PAGE_2;
        } else if (inv_item->current_frame == 29) {
            inv_item->meshes_drawn = PM_COMMON;
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

static void M_UpdateInventoryItem(
    const INV_RING *const ring, INV_ITEM *const inv_item,
    const int32_t num_frames)
{
    if (ring->motion.status == RNG_DONE
        || inv_item != ring->list[ring->current_object]) {
        for (int32_t i = 0; i < num_frames; i++) {
            if (inv_item->y_rot < 0) {
                inv_item->y_rot += 256;
            } else if (inv_item->y_rot > 0) {
                inv_item->y_rot -= 256;
            }
        }
    } else if (ring->rotating) {
        for (int32_t i = 0; i < num_frames; i++) {
            if (inv_item->y_rot > 0) {
                inv_item->y_rot -= 512;
            } else if (inv_item->y_rot < 0) {
                inv_item->y_rot += 512;
            }
        }
    } else if (
        ring->motion.status == RNG_SELECTED
        || ring->motion.status == RNG_DESELECTING
        || ring->motion.status == RNG_SELECTING
        || ring->motion.status == RNG_DESELECT
        || ring->motion.status == RNG_CLOSING_ITEM) {
        for (int32_t i = 0; i < num_frames; i++) {
            const int32_t delta = inv_item->y_rot_sel - inv_item->y_rot;
            if (delta != 0) {
                if (delta > 0 && delta < PHD_180) {
                    inv_item->y_rot += 1024;
                } else {
                    inv_item->y_rot -= 1024;
                }
                inv_item->y_rot &= ~(1024 - 1);
            }
        }
    } else if (
        ring->number_of_objects == 1 || (!g_Input.right && !g_Input.left)) {
        for (int32_t i = 0; i < num_frames; i++) {
            inv_item->y_rot += 256;
        }
    }
}

static bool M_AnimateInventoryItem(INV_ITEM *const inv_item)
{
    if (inv_item->current_frame == inv_item->goal_frame) {
        M_SelectMeshes(inv_item);
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

    M_SelectMeshes(inv_item);
    return true;
}

INV_RING *InvRing_Open(const INVENTORY_MODE mode)
{
    if (mode == INV_KEYS_MODE && g_Inv_KeyObjectsCount == 0) {
        g_Inv_Chosen = NO_OBJECT;
        return NULL;
    }

    Clock_SyncTick();
    Output_SetupAboveWater(false);

    g_PhdWinRight = g_PhdWinMaxX;
    g_PhdWinLeft = 0;
    g_PhdWinTop = 0;
    g_PhdWinBottom = g_PhdWinMaxY;
    g_Inv_Chosen = NO_OBJECT;

    if (g_CurrentLevel != LV_GYM) {
        Stats_StartTimer();
    }

    if (mode == INV_TITLE_MODE) {
        Output_LoadBackgroundFromFile("data/title.pcx");
    } else {
        Output_LoadBackgroundFromObject();
    }
    Overlay_HideGameInfo();
    Output_AlterFOV(80 * PHD_DEGREE);

    Sound_StopAllSamples();
    if (mode != INV_TITLE_MODE) {
        Music_Pause();
    }

    M_Construct(mode);

    INV_RING *const ring = Memory_Alloc(sizeof(INV_RING));
    ring->mode = mode;
    ring->pass_open = false;
    ring->demo_needed = false;
    m_NoInputCounter = 0;

    switch (mode) {
    case INV_TITLE_MODE:
    case INV_SAVE_MODE:
    case INV_LOAD_MODE:
    case INV_DEATH_MODE:
        InvRing_InitRing(
            ring, 1, g_Inv_OptionList, g_Inv_OptionObjectsCount,
            g_Inv_OptionCurrent);
        break;

    case INV_KEYS_MODE:
        InvRing_InitRing(
            ring, 2, g_Inv_KeysList, g_Inv_KeyObjectsCount, g_Inv_MainCurrent);
        break;

    default:
        if (g_Inv_MainObjectsCount) {
            InvRing_InitRing(
                ring, 0, g_Inv_MainList, g_Inv_MainObjectsCount,
                g_Inv_MainCurrent);
        } else {
            InvRing_InitRing(
                ring, 1, g_Inv_OptionList, g_Inv_OptionObjectsCount,
                g_Inv_OptionCurrent);
        }
        break;
    }

    Sound_Effect(SFX_MENU_SPININ, 0, SPM_ALWAYS);
    g_Inv_Mode = mode;
    return ring;
}

GAME_FLOW_DIR InvRing_Close(INV_RING *const ring)
{
    const INVENTORY_MODE mode = ring->mode;
    const bool demo_needed = ring->demo_needed;
    M_End(ring);

    // enable buffering
    g_OldInputDB = (INPUT_STATE) { 0 };

    if (g_IsGameToExit) {
        return GFD_EXIT_GAME;
    } else if (g_GF_OverrideDir != (GAME_FLOW_DIR)-1) {
        return GFD_OVERRIDE;
    } else if (demo_needed) {
        return GFD_START_DEMO | 0xFF;
    } else if (g_Inv_Chosen == NO_OBJECT) {
        if (mode != INV_TITLE_MODE && g_Config.audio.music_volume != 0) {
            Music_Unpause();
        }
    } else {
        if (mode != INV_TITLE_MODE) {
            Music_Unpause();
        }
        if (g_Config.gameplay.fix_item_duplication_glitch) {
            InvRing_ClearSelection();
        }

        switch (g_Inv_Chosen) {
        case O_PASSPORT_OPTION:
            if (g_Inv_ExtraData[0] == 1 && g_Config.audio.music_volume != 0) {
                Music_Unpause();
            }

            if (g_Inv_ExtraData[0] == 0) {
                // first passport page: load game.
                Inv_RemoveAllItems();
                S_LoadGame(
                    &g_SaveGame, sizeof(SAVEGAME_INFO), g_Inv_ExtraData[1]);
                return GFD_START_SAVED_GAME | g_Inv_ExtraData[1];
            } else if (g_Inv_ExtraData[0] == 1) {
                // second passport page:
                if (mode == INV_TITLE_MODE) {
                    // title mode - new game or select level.
                    if (g_GameFlow.play_any_level) {
                        return g_Inv_ExtraData[1] + 1;
                    } else {
                        InitialiseStartInfo();
                        return GFD_START_GAME | LV_FIRST;
                    }
                } else {
                    // game mode - save game (or start the game if in Lara's
                    // Home)
                    if (g_CurrentLevel == LV_GYM) {
                        InitialiseStartInfo();
                        return GFD_START_GAME | LV_FIRST;
                    } else {
                        CreateSaveGameInfo();
                        const int16_t slot_num = g_Inv_ExtraData[1];
                        S_SaveGame(
                            &g_SaveGame, sizeof(SAVEGAME_INFO), slot_num);
                    }
                }
            } else {
                // third passport page:
                if (mode == INV_TITLE_MODE) {
                    // title mode - exit the game
                    return GFD_EXIT_GAME;
                } else {
                    // game mode - exit to title
                    return GFD_EXIT_TO_TITLE;
                }
            }
            break;

        case O_PHOTO_OPTION:
            if (g_GameFlow.gym_enabled) {
                return GFD_START_GAME | LV_GYM;
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

    return (GAME_FLOW_DIR)-1;
}

GAME_FLOW_DIR InvRing_Control(INV_RING *const ring, const int32_t num_frames)
{
    if (g_GF_OverrideDir != (GAME_FLOW_DIR)-1) {
        return GFD_OVERRIDE;
    }

    InvRing_CalcAdders(ring, 24);
    Shell_ProcessEvents();
    Input_Update();
    Shell_ProcessInput();
    Game_ProcessInput();

    if (g_Inv_DemoMode) {
        if (g_InputDB.any) {
            return g_GameFlow.on_demo_interrupt;
        }
        if (!Demo_GetInput()) {
            return g_GameFlow.on_demo_end;
        }
    } else if (ring->mode != INV_TITLE_MODE || g_Input.any || g_InputDB.any) {
        m_NoInputCounter = 0;
    } else if (g_GameFlow.num_demos > 0 && ring->motion.status == RNG_OPEN) {
        m_NoInputCounter++;
        if (m_NoInputCounter > g_GameFlow.no_input_time) {
            ring->demo_needed = true;
        }
    }

    if (g_IsGameToExit) {
        return GFD_EXIT_GAME;
    }

    if ((ring->mode == INV_SAVE_MODE || ring->mode == INV_LOAD_MODE
         || ring->mode == INV_DEATH_MODE)
        && !ring->pass_open) {
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0, .menu_confirm = 1 };
    }

    for (int32_t frame = 0; frame < num_frames; frame++) {
        if (g_Inv_IsOptionsDelay) {
            if (g_Inv_OptionsDelayCounter) {
                g_Inv_OptionsDelayCounter--;
            } else {
                g_Inv_IsOptionsDelay = 0;
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

            if (ring->demo_needed
                || ((g_InputDB.option || g_InputDB.menu_back)
                    && ring->mode != INV_TITLE_MODE)) {
                Sound_Effect(SFX_MENU_SPINOUT, 0, SPM_ALWAYS);
                g_Inv_Chosen = NO_OBJECT;
                if (ring->type != RT_MAIN) {
                    g_Inv_OptionCurrent = ring->current_object;
                } else {
                    g_Inv_MainCurrent = ring->current_object;
                }
                InvRing_MotionSetup(ring, RNG_CLOSING, RNG_DONE, 32);
                InvRing_MotionRadius(ring, 0);
                InvRing_MotionCameraPos(ring, -1536);
                InvRing_MotionRotation(
                    ring, PHD_180, ring->ring_pos.rot.y + PHD_180);
                g_Input = (INPUT_STATE) { 0 };
                g_InputDB = (INPUT_STATE) { 0 };
            }

            if (g_InputDB.menu_confirm) {
                if ((ring->mode == INV_SAVE_MODE || ring->mode == INV_LOAD_MODE
                     || ring->mode == INV_DEATH_MODE)
                    && !ring->pass_open) {
                    ring->pass_open = true;
                }

                g_SoundOptionLine = 0;
                INV_ITEM *inv_item;
                if (ring->type == RT_MAIN) {
                    g_Inv_MainCurrent = ring->current_object;
                    inv_item = g_Inv_MainList[ring->current_object];
                } else if (ring->type == RT_OPTION) {
                    g_Inv_OptionCurrent = ring->current_object;
                    inv_item = g_Inv_OptionList[ring->current_object];
                } else {
                    g_Inv_KeysCurrent = ring->current_object;
                    inv_item = g_Inv_KeysList[ring->current_object];
                }

                inv_item->goal_frame = inv_item->open_frame;
                inv_item->anim_direction = 1;
                InvRing_MotionSetup(ring, RNG_SELECTING, RNG_SELECTED, 16);
                InvRing_MotionRotation(
                    ring, 0, -16384 - ring->angle_adder * ring->current_object);
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
                    if (g_Inv_MainObjectsCount > 0) {
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
                    if (g_Inv_KeyObjectsCount > 0) {
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
                    if (g_Inv_MainObjectsCount > 0) {
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
                    if (g_Inv_OptionObjectsCount > 0
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
            InvRing_MotionRadius(ring, 688);
            ring->camera_pitch = -(int16_t)(ring->motion.misc);
            ring->motion.camera_pitch_rate = ring->motion.misc / 24;
            ring->motion.camera_pitch_target = 0;
            ring->list = g_Inv_OptionList;
            ring->type = RT_OPTION;
            g_Inv_MainCurrent = ring->current_object;
            g_Inv_MainObjectsCount = ring->number_of_objects;
            ring->number_of_objects = g_Inv_OptionObjectsCount;
            ring->current_object = g_Inv_OptionCurrent;
            InvRing_CalcAdders(ring, 24);
            InvRing_MotionRotation(
                ring, PHD_180,
                -16384 - ring->angle_adder * ring->current_object);
            ring->ring_pos.rot.y = ring->motion.rotate_target + PHD_180;
            break;

        case RNG_MAIN2KEYS:
            InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, 24);
            InvRing_MotionRadius(ring, 688);
            ring->motion.camera_pitch_target = 0;
            ring->camera_pitch = -(int16_t)(ring->motion.misc);
            ring->motion.camera_pitch_rate = ring->motion.misc / 24;
            g_Inv_MainCurrent = ring->current_object;
            g_Inv_MainObjectsCount = ring->number_of_objects;
            ring->list = g_Inv_KeysList;
            ring->type = RT_KEYS;
            ring->number_of_objects = g_Inv_KeyObjectsCount;
            ring->current_object = g_Inv_KeysCurrent;
            InvRing_CalcAdders(ring, 24);
            InvRing_MotionRotation(
                ring, PHD_180,
                -16384 - ring->angle_adder * ring->current_object);
            ring->ring_pos.rot.y = ring->motion.rotate_target + PHD_180;
            break;

        case RNG_KEYS2MAIN:
            InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, 24);
            InvRing_MotionRadius(ring, 688);
            ring->camera_pitch = -(int16_t)(ring->motion.misc);
            ring->motion.camera_pitch_rate = ring->motion.misc / 24;
            ring->motion.camera_pitch_target = 0;
            ring->list = g_Inv_MainList;
            ring->type = RT_MAIN;
            g_Inv_KeysCurrent = ring->current_object;
            ring->number_of_objects = g_Inv_MainObjectsCount;
            ring->current_object = g_Inv_MainCurrent;
            InvRing_CalcAdders(ring, 24);
            InvRing_MotionRotation(
                ring, PHD_180,
                -16384 - ring->angle_adder * ring->current_object);
            ring->ring_pos.rot.y = ring->motion.rotate_target + PHD_180;
            break;

        case RNG_OPTION2MAIN:
            InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, 24);
            InvRing_MotionRadius(ring, 688);
            ring->camera_pitch = -(int16_t)(ring->motion.misc);
            ring->motion.camera_pitch_rate = ring->motion.misc / 24;
            g_Inv_OptionCurrent = ring->current_object;
            g_Inv_OptionObjectsCount = ring->number_of_objects;
            ring->motion.camera_pitch_target = 0;
            ring->list = g_Inv_MainList;
            ring->type = RT_MAIN;
            ring->number_of_objects = g_Inv_MainObjectsCount;
            ring->current_object = g_Inv_MainCurrent;
            InvRing_CalcAdders(ring, 24);
            InvRing_MotionRotation(
                ring, PHD_180,
                -16384 - ring->angle_adder * ring->current_object);
            ring->ring_pos.rot.y = ring->motion.rotate_target + PHD_180;
            break;

        case RNG_SELECTED: {
            INV_ITEM *inv_item = ring->list[ring->current_object];
            if (inv_item->object_id == O_PASSPORT_CLOSED) {
                inv_item->object_id = O_PASSPORT_OPTION;
            }

            bool busy = false;
            for (int32_t frame = 0; frame < num_frames; frame++) {
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
                        g_Inv_OptionCurrent = ring->current_object;
                    } else {
                        g_Inv_MainCurrent = ring->current_object;
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
                ring, 0, -16384 - ring->angle_adder * ring->current_object);
            g_Input = (INPUT_STATE) { 0 };
            g_InputDB = (INPUT_STATE) { 0 };
            break;

        case RNG_CLOSING_ITEM: {
            INV_ITEM *inv_item = ring->list[ring->current_object];
            for (int32_t frame = 0; frame < num_frames; frame++) {
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
            if (!ring->motion.count) {
                InvRing_MotionSetup(ring, RNG_CLOSING, RNG_DONE, 32);
                InvRing_MotionRadius(ring, 0);
                InvRing_MotionCameraPos(ring, -1536);
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
            INV_ITEM *const inv_item = ring->list[ring->current_object];
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
        M_UpdateInventoryItem(ring, ring->list[i], num_frames);
    }

    Sound_EndScene();
    Output_AnimateTextures(num_frames);
    Overlay_Animate(num_frames / 2);
    g_Camera.num_frames = num_frames;
    return (GAME_FLOW_DIR)-1;
}

void InvRing_ClearSelection(void)
{
    g_Inv_MainCurrent = 0;
    g_Inv_KeysCurrent = 0;
}
