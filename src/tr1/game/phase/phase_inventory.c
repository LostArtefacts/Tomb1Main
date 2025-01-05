#include "game/phase/phase_inventory.h"

#include "game/console/common.h"
#include "game/game.h"
#include "game/gameflow.h"
#include "game/input.h"
#include "game/interpolation.h"
#include "game/inventory.h"
#include "game/inventory_ring.h"
#include "game/inventory_ring/priv.h"
#include "game/lara/common.h"
#include "game/music.h"
#include "game/option.h"
#include "game/option/option_compass.h"
#include "game/output.h"
#include "game/savegame.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "game/viewport.h"
#include "global/vars.h"

#include <libtrx/config.h>

typedef enum {
    PSPINE = 1,
    PFRONT = 2,
    PINFRONT = 4,
    PPAGE2 = 8,
    PBACK = 16,
    PINBACK = 32,
    PPAGE1 = 64
} PASS_PAGE;

static bool m_PlayedSpinin;
static bool m_PassportModeReady;
static int32_t m_StartLevel;
static bool m_StartDemo;
static TEXTSTRING *m_VersionText = NULL;
static int32_t m_StatsCounter;
static GAME_OBJECT_ID m_InvChosen;
static CLOCK_TIMER m_DemoTimer = { 0 };
static CLOCK_TIMER m_StatsTimer = { 0 };
RING_INFO m_Ring;
IMOTION_INFO m_Motion;

static void InvRing_Construct(void);
static void InvRing_Destroy(void);
static PHASE_CONTROL InvRing_Close(GAME_OBJECT_ID inv_chosen);
static void InvRing_SelectMeshes(INVENTORY_ITEM *inv_item);
static bool InvRing_AnimateItem(INVENTORY_ITEM *inv_item);
static bool InvRing_CheckDemoTimer(const IMOTION_INFO *motion);

static void M_Start(const PHASE_INVENTORY_ARGS *args);
static void M_End(void);
static PHASE_CONTROL M_ControlFrame(void);
static PHASE_CONTROL M_Control(int32_t nframes);
static void M_Draw(void);

static void InvRing_Construct(void)
{
    g_PhdLeft = Viewport_GetMinX();
    g_PhdTop = Viewport_GetMinY();
    g_PhdBottom = Viewport_GetMaxY();
    g_PhdRight = Viewport_GetMaxX();

    m_InvChosen = NO_OBJECT;
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

static void InvRing_Destroy(void)
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

static PHASE_CONTROL InvRing_Close(GAME_OBJECT_ID inv_chosen)
{
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

    if (m_StartDemo) {
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

static void InvRing_SelectMeshes(INVENTORY_ITEM *inv_item)
{
    if (inv_item->object_id == O_PASSPORT_OPTION) {
        if (inv_item->current_frame <= 14) {
            inv_item->drawn_meshes = PASS_MESH | PINFRONT | PPAGE1;
        } else if (inv_item->current_frame < 19) {
            inv_item->drawn_meshes = PASS_MESH | PINFRONT | PPAGE1 | PPAGE2;
        } else if (inv_item->current_frame == 19) {
            inv_item->drawn_meshes = PASS_MESH | PPAGE1 | PPAGE2;
        } else if (inv_item->current_frame < 24) {
            inv_item->drawn_meshes = PASS_MESH | PPAGE1 | PPAGE2 | PINBACK;
        } else if (inv_item->current_frame < 29) {
            inv_item->drawn_meshes = PASS_MESH | PPAGE2 | PINBACK;
        } else if (inv_item->current_frame == 29) {
            inv_item->drawn_meshes = PASS_MESH;
        }
    } else if (inv_item->object_id == O_MAP_OPTION) {
        if (inv_item->current_frame && inv_item->current_frame < 18) {
            inv_item->drawn_meshes = -1;
        } else {
            inv_item->drawn_meshes = inv_item->which_meshes;
        }
    } else {
        inv_item->drawn_meshes = -1;
    }
}

static bool InvRing_AnimateItem(INVENTORY_ITEM *inv_item)
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

static bool InvRing_CheckDemoTimer(const IMOTION_INFO *const motion)
{
    if (!g_Config.gameplay.enable_demo || !g_GameFlow.has_demo) {
        return false;
    }

    if (g_InvMode != INV_TITLE_MODE || g_Input.any || g_InputDB.any
        || Console_IsOpened()) {
        Clock_ResetTimer(&m_DemoTimer);
        return false;
    }

    return motion->status == RNG_OPEN
        && Clock_CheckElapsedMilliseconds(
               &m_DemoTimer, g_GameFlow.demo_delay * 1000.0);
}

static void M_Start(const PHASE_INVENTORY_ARGS *const args)
{
    Interpolation_Remember();
    if (g_Config.gameplay.enable_timer_in_inventory) {
        Stats_StartTimer();
    }

    RING_INFO *ring = &m_Ring;
    IMOTION_INFO *motion = &m_Motion;

    memset(motion, 0, sizeof(IMOTION_INFO));
    memset(ring, 0, sizeof(RING_INFO));

    g_InvMode = args->mode;

    m_PassportModeReady = false;
    m_StartLevel = -1;
    m_StartDemo = false;
    InvRing_Construct();

    if (!g_Config.audio.enable_music_in_inventory
        && g_InvMode != INV_TITLE_MODE) {
        Music_Pause();
        Sound_PauseAll();
    } else {
        Sound_ResetAmbient();
        Sound_UpdateEffects();
    }

    switch (g_InvMode) {
    case INV_DEATH_MODE:
    case INV_SAVE_MODE:
    case INV_SAVE_CRYSTAL_MODE:
    case INV_LOAD_MODE:
    case INV_TITLE_MODE:
        InvRing_InitRing(
            ring, RT_OPTION, g_InvOptionList, g_InvOptionObjects,
            g_InvOptionCurrent, motion);
        break;

    case INV_KEYS_MODE:
        InvRing_InitRing(
            ring, RT_KEYS, g_InvKeysList, g_InvKeysObjects, g_InvMainCurrent,
            motion);
        break;

    default:
        if (g_InvMainObjects) {
            InvRing_InitRing(
                ring, RT_MAIN, g_InvMainList, g_InvMainObjects,
                g_InvMainCurrent, motion);
        } else {
            InvRing_InitRing(
                ring, RT_OPTION, g_InvOptionList, g_InvOptionObjects,
                g_InvOptionCurrent, motion);
        }
        break;
    }

    m_PlayedSpinin = false;
    g_InvRing_OldCamera = g_Camera;

    if (g_InvMode == INV_TITLE_MODE) {
        Output_FadeResetToBlack();
        Output_FadeToTransparent(true);
    } else {
        Output_FadeToSemiBlack(true);
    }
}

static PHASE_CONTROL M_ControlFrame(void)
{
    RING_INFO *ring = &m_Ring;
    IMOTION_INFO *motion = &m_Motion;

    if (motion->status == RNG_OPENING) {
        if (g_InvMode == INV_TITLE_MODE && Output_FadeIsAnimating()) {
            return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
        }

        Clock_ResetTimer(&m_DemoTimer);
        if (!m_PlayedSpinin) {
            Sound_Effect(SFX_MENU_SPININ, NULL, SPM_ALWAYS);
            m_PlayedSpinin = true;
        }
    }

    if (motion->status == RNG_DONE) {
        // finish fading
        if (g_InvMode == INV_TITLE_MODE) {
            Output_FadeToBlack(true);
        }

        if (Output_FadeIsAnimating()) {
            return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
        }

        return InvRing_Close(m_InvChosen);
    }

    InvRing_CalcAdders(ring, ROTATE_DURATION);

    Input_Update();
    // Do the demo inactivity check prior to postprocessing of the inputs.
    if (InvRing_CheckDemoTimer(motion)) {
        m_StartDemo = true;
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
        && !m_PassportModeReady) {
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0, .menu_confirm = 1 };
    }

    if (!(g_InvMode == INV_TITLE_MODE || Output_FadeIsAnimating()
          || motion->status == RNG_OPENING)) {
        for (int i = 0; i < ring->number_of_objects; i++) {
            INVENTORY_ITEM *inv_item = ring->list[i];
            if (inv_item->object_id == O_MAP_OPTION) {
                Option_Compass_UpdateNeedle(inv_item);
            }
        }
    }

    switch (motion->status) {
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

        if (m_StartLevel != -1 || m_StartDemo
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
                ring, CLOSE_ROTATION, ring->ringpos.rot.y - CLOSE_ROTATION);
            g_Input = (INPUT_STATE) { 0 };
            g_InputDB = (INPUT_STATE) { 0 };
        }

        const bool examine = g_InputDB.look && InvRing_CanExamine();
        if (g_InputDB.menu_confirm || examine) {
            if ((g_InvMode == INV_SAVE_MODE
                 || g_InvMode == INV_SAVE_CRYSTAL_MODE
                 || g_InvMode == INV_LOAD_MODE || g_InvMode == INV_DEATH_MODE)
                && !m_PassportModeReady) {
                m_PassportModeReady = true;
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
                        ring->ringpos.rot.y - CLOSE_ROTATION);
                    InvRing_MotionCameraPitch(ring, 0x2000);
                    motion->misc = 0x2000;
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
                        ring->ringpos.rot.y - CLOSE_ROTATION);
                    InvRing_MotionCameraPitch(ring, 0x2000);
                    motion->misc = 0x2000;
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
                        ring->ringpos.rot.y - CLOSE_ROTATION);
                    InvRing_MotionCameraPitch(ring, -0x2000);
                    motion->misc = -0x2000;
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
                        ring->ringpos.rot.y - CLOSE_ROTATION);
                    InvRing_MotionCameraPitch(ring, -0x2000);
                    motion->misc = -0x2000;
                }
                g_InputDB = (INPUT_STATE) { 0 };
            }
        }
        break;

    case RNG_MAIN2OPTION:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, RING_RADIUS);
        ring->camera_pitch = -motion->misc;
        motion->camera_pitch_rate = motion->misc / (RINGSWITCH_FRAMES / 2);
        motion->camera_pitch_target = 0;
        g_InvMainCurrent = ring->current_object;
        ring->list = g_InvOptionList;
        ring->type = RT_OPTION;
        ring->number_of_objects = g_InvOptionObjects;
        ring->current_object = g_InvOptionCurrent;
        InvRing_CalcAdders(ring, ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, OPEN_ROTATION,
            -PHD_90 - ring->angle_adder * ring->current_object);
        ring->ringpos.rot.y = motion->rotate_target + OPEN_ROTATION;
        break;

    case RNG_MAIN2KEYS:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, RING_RADIUS);
        ring->camera_pitch = -motion->misc;
        motion->camera_pitch_rate = motion->misc / (RINGSWITCH_FRAMES / 2);
        motion->camera_pitch_target = 0;
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
        ring->ringpos.rot.y = motion->rotate_target + OPEN_ROTATION;
        break;

    case RNG_KEYS2MAIN:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, RING_RADIUS);
        ring->camera_pitch = -motion->misc;
        motion->camera_pitch_rate = motion->misc / (RINGSWITCH_FRAMES / 2);
        motion->camera_pitch_target = 0;
        g_InvKeysCurrent = ring->current_object;
        ring->list = g_InvMainList;
        ring->type = RT_MAIN;
        ring->number_of_objects = g_InvMainObjects;
        ring->current_object = g_InvMainCurrent;
        InvRing_CalcAdders(ring, ROTATE_DURATION);
        InvRing_MotionRotation(
            ring, OPEN_ROTATION,
            -PHD_90 - ring->angle_adder * ring->current_object);
        ring->ringpos.rot.y = motion->rotate_target + OPEN_ROTATION;
        break;

    case RNG_OPTION2MAIN:
        InvRing_MotionSetup(ring, RNG_OPENING, RNG_OPEN, RINGSWITCH_FRAMES / 2);
        InvRing_MotionRadius(ring, RING_RADIUS);
        ring->camera_pitch = -motion->misc;
        motion->camera_pitch_rate = motion->misc / (RINGSWITCH_FRAMES / 2);
        motion->camera_pitch_target = 0;
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
        ring->ringpos.rot.y = motion->rotate_target + OPEN_ROTATION;
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
                inv_item->sprlist = NULL;
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
                inv_item->sprlist = NULL;
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
            motion->count = SELECTING_FRAMES;
            motion->status = motion->status_target;
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

        if (!motion->count) {
            InvRing_MotionSetup(ring, RNG_CLOSING, RNG_DONE, CLOSE_FRAMES);
            InvRing_MotionRadius(ring, 0);
            InvRing_MotionCameraPos(ring, CAMERA_STARTHEIGHT);
            InvRing_MotionRotation(
                ring, CLOSE_ROTATION, ring->ringpos.rot.y - CLOSE_ROTATION);
        }
        break;
    }

    if (motion->status == RNG_OPEN || motion->status == RNG_SELECTING
        || motion->status == RNG_SELECTED || motion->status == RNG_DESELECTING
        || motion->status == RNG_DESELECT
        || motion->status == RNG_CLOSING_ITEM) {
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

    if (!motion->status || motion->status == RNG_CLOSING
        || motion->status == RNG_MAIN2OPTION
        || motion->status == RNG_OPTION2MAIN
        || motion->status == RNG_EXITING_INVENTORY || motion->status == RNG_DONE
        || ring->rotating) {
        InvRing_RemoveAllText();
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static PHASE_CONTROL M_Control(int32_t nframes)
{
    Interpolation_Remember();
    if (g_Config.gameplay.enable_timer_in_inventory) {
        Stats_UpdateTimer();
    }
    for (int32_t i = 0; i < nframes; i++) {
        const PHASE_CONTROL result = M_ControlFrame();
        if (result.action == PHASE_ACTION_END) {
            return result;
        }
    }

    return (PHASE_CONTROL) { .action = PHASE_ACTION_CONTINUE };
}

static void M_End(void)
{
    INVENTORY_ITEM *const inv_item = m_Ring.list[m_Ring.current_object];
    if (inv_item != NULL) {
        Option_Shutdown(inv_item);
    }

    InvRing_Destroy();
    if (g_Config.input.enable_buffering) {
        g_OldInputDB = (INPUT_STATE) { 0 };
    }
    if (g_InvMode == INV_TITLE_MODE) {
        Music_Stop();
        Sound_StopAllSamples();
    }
}

static void M_Draw(void)
{
    RING_INFO *ring = &m_Ring;
    IMOTION_INFO *motion = &m_Motion;
    InvRing_Draw(ring, motion);
    Output_AnimateFades();
    Text_Draw();
}

PHASER g_InventoryPhaser = {
    .start = (PHASER_START)M_Start,
    .end = M_End,
    .control = M_Control,
    .draw = M_Draw,
    .wait = NULL,
};
