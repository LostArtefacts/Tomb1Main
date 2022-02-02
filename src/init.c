#include "game/gameflow.h"
#include "global/vars.h"
#include "memory.h"

// THIS IS AN AUTOGENERATED FILE.
// To generate it, run scripts/generate_init.

#ifndef VERSION
    #define VERSION "T1M (unknown version)"
#endif

const char *g_T1MVersion = VERSION;

GAMEFLOW_DEFAULT_STRING g_GameFlowDefaultStrings[] = {
    { GS_HEADING_INVENTORY, "INVENTORY" },
    { GS_HEADING_GAME_OVER, "GAME OVER" },
    { GS_HEADING_OPTION, "OPTION" },
    { GS_HEADING_ITEMS, "ITEMS" },
    { GS_PASSPORT_SELECT_LEVEL, "Select Level" },
    { GS_PASSPORT_SELECT_MODE, "Select Mode" },
    { GS_PASSPORT_MODE_NEW_GAME, "New Game" },
    { GS_PASSPORT_MODE_NEW_GAME_PLUS, "New Game+" },
    { GS_PASSPORT_MODE_NEW_GAME_JP, "Japanese NG" },
    { GS_PASSPORT_MODE_NEW_GAME_JP_PLUS, "Japanese NG+" },
    { GS_PASSPORT_NEW_GAME, "New Game" },
    { GS_PASSPORT_LOAD_GAME, "Load Game" },
    { GS_PASSPORT_SAVE_GAME, "Save Game" },
    { GS_PASSPORT_EXIT_GAME, "Exit Game" },
    { GS_PASSPORT_EXIT_TO_TITLE, "Exit to Title" },
    { GS_DETAIL_SELECT_DETAIL, "Select Detail" },
    { GS_DETAIL_LEVEL_HIGH, "High" },
    { GS_DETAIL_LEVEL_MEDIUM, "Medium" },
    { GS_DETAIL_LEVEL_LOW, "Low" },
    { GS_DETAIL_PERSPECTIVE_FMT, "Perspective     %s" },
    { GS_DETAIL_BILINEAR_FMT, "Bilinear        %s" },
    { GS_DETAIL_VSYNC_FMT, "VSync           %s" },
    { GS_DETAIL_BRIGHTNESS_FMT, "Brightness      %.1f" },
    { GS_DETAIL_UI_TEXT_SCALE_FMT, "UI text scale   %.1f" },
    { GS_DETAIL_UI_BAR_SCALE_FMT, "UI bar scale    %.1f" },
    { GS_DETAIL_VIDEO_MODE_FMT, "Game Video Mode %s" },
    { GS_SOUND_SET_VOLUMES, "Set Volumes" },
    { GS_CONTROL_DEFAULT_KEYS, "Default Keys" },
    { GS_CONTROL_USER_KEYS, "User Keys" },
    { GS_KEYMAP_RUN, "Run" },
    { GS_KEYMAP_BACK, "Back" },
    { GS_KEYMAP_LEFT, "Left" },
    { GS_KEYMAP_RIGHT, "Right" },
    { GS_KEYMAP_STEP_LEFT, "Step Left" },
    { GS_KEYMAP_STEP_RIGHT, "Step Right" },
    { GS_KEYMAP_WALK, "Walk" },
    { GS_KEYMAP_JUMP, "Jump" },
    { GS_KEYMAP_ACTION, "Action" },
    { GS_KEYMAP_DRAW_WEAPON, "Draw Weapon" },
    { GS_KEYMAP_LOOK, "Look" },
    { GS_KEYMAP_ROLL, "Roll" },
    { GS_KEYMAP_INVENTORY, "Inventory" },
    { GS_KEYMAP_PAUSE, "Pause" },
    { GS_KEYMAP_FLY_CHEAT, "Fly cheat" },
    { GS_KEYMAP_ITEM_CHEAT, "Item cheat" },
    { GS_KEYMAP_LEVEL_SKIP_CHEAT, "Level skip" },
    { GS_KEYMAP_CAMERA_UP, "Camera Up" },
    { GS_KEYMAP_CAMERA_DOWN, "Camera Down" },
    { GS_KEYMAP_CAMERA_LEFT, "Camera Left" },
    { GS_KEYMAP_CAMERA_RIGHT, "Camera Right" },
    { GS_KEYMAP_CAMERA_RESET, "Reset Camera" },
    { GS_STATS_TIME_TAKEN_FMT, "TIME TAKEN %s" },
    { GS_STATS_SECRETS_FMT, "SECRETS %d OF %d" },
    { GS_STATS_PICKUPS_FMT, "PICKUPS %d OF %d" },
    { GS_STATS_KILLS_FMT, "KILLS %d OF %d" },
    { GS_PAUSE_PAUSED, "Paused" },
    { GS_PAUSE_EXIT_TO_TITLE, "Exit to title?" },
    { GS_PAUSE_CONTINUE, "Continue" },
    { GS_PAUSE_QUIT, "Quit" },
    { GS_PAUSE_ARE_YOU_SURE, "Are you sure?" },
    { GS_PAUSE_YES, "Yes" },
    { GS_PAUSE_NO, "No" },
    { GS_MISC_ON, "On" },
    { GS_MISC_OFF, "Off" },
    { GS_MISC_EMPTY_SLOT_FMT, "- EMPTY SLOT %d -" },
    { GS_MISC_DEMO_MODE, "Demo Mode" },
    { GS_INV_ITEM_MEDI, "Small Medi Pack" },
    { GS_INV_ITEM_BIG_MEDI, "Large Medi Pack" },
    { GS_INV_ITEM_PUZZLE1, "Puzzle" },
    { GS_INV_ITEM_PUZZLE2, "Puzzle" },
    { GS_INV_ITEM_PUZZLE3, "Puzzle" },
    { GS_INV_ITEM_PUZZLE4, "Puzzle" },
    { GS_INV_ITEM_KEY1, "Key" },
    { GS_INV_ITEM_KEY2, "Key" },
    { GS_INV_ITEM_KEY3, "Key" },
    { GS_INV_ITEM_KEY4, "Key" },
    { GS_INV_ITEM_PICKUP1, "Pickup" },
    { GS_INV_ITEM_PICKUP2, "Pickup" },
    { GS_INV_ITEM_LEADBAR, "Lead Bar" },
    { GS_INV_ITEM_SCION, "Scion" },
    { GS_INV_ITEM_PISTOLS, "Pistols" },
    { GS_INV_ITEM_SHOTGUN, "Shotgun" },
    { GS_INV_ITEM_MAGNUM, "Magnums" },
    { GS_INV_ITEM_UZI, "Uzis" },
    { GS_INV_ITEM_GRENADE, "Grenade" },
    { GS_INV_ITEM_PISTOL_AMMO, "Pistol Clips" },
    { GS_INV_ITEM_SHOTGUN_AMMO, "Shotgun Shells" },
    { GS_INV_ITEM_MAGNUM_AMMO, "Magnum Clips" },
    { GS_INV_ITEM_UZI_AMMO, "Uzi Clips" },
    { GS_INV_ITEM_COMPASS, "Compass" },
    { GS_INV_ITEM_GAME, "Game" },
    { GS_INV_ITEM_DETAILS, "Detail Levels" },
    { GS_INV_ITEM_SOUND, "Sound" },
    { GS_INV_ITEM_CONTROLS, "Controls" },
    { GS_INV_ITEM_LARAS_HOME, "Lara's Home" },
    { 0, NULL },
};
