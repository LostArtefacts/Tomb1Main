#include "decomp/decomp.h"
#include "decomp/savegame.h"
#include "game/input.h"
#include "game/inventory_ring.h"
#include "game/option/option.h"
#include "game/requester.h"
#include "game/sound.h"
#include "game/text.h"
#include "global/vars.h"

typedef enum {
    PASSPORT_MODE_BROWSE = 0,
    PASSPORT_MODE_LOAD_GAME = 1,
    PASSPORT_MODE_SELECT_LEVEL = 2,
} PASSPORT_MODE;

static TEXTSTRING *m_LevelText = NULL;
static PASSPORT_MODE m_PassportMode;

void Option_Passport_Control(INVENTORY_ITEM *const item)
{
    InvRing_RemoveAllText();

    const int32_t frame = item->goal_frame - item->open_frame;
    const int32_t page = frame % 5 == 0 ? frame / 5 : -1;

    if (g_Inv_Mode == INV_LOAD_MODE || g_Inv_Mode == INV_SAVE_MODE
        || g_GameFlow.load_save_disabled) {
        g_InputDB.menu_left = 0;
        g_InputDB.menu_right = 0;
    }

    switch (page) {
    case 0:
        if (g_GameFlow.load_save_disabled) {
            g_InputDB = (INPUT_STATE) { .menu_right = 1 };
            break;
        }

        if (m_PassportMode == PASSPORT_MODE_LOAD_GAME) {
            Requester_SetSize(&g_LoadGameRequester, 10, -32);

            const int32_t select =
                Requester_Display(&g_LoadGameRequester, 1, 1);
            if (select != 0) {
                if (select > 0) {
                    g_Inv_ExtraData[1] = select - 1;
                }
                m_PassportMode = PASSPORT_MODE_BROWSE;
            } else if (g_InputDB.menu_right) {
                Requester_Shutdown(&g_LoadGameRequester);
                m_PassportMode = PASSPORT_MODE_BROWSE;
            } else {
                g_Input = (INPUT_STATE) {};
                g_InputDB = (INPUT_STATE) {};
            }
        } else if (m_PassportMode == PASSPORT_MODE_BROWSE) {
            if (g_SavedGames != 0 && g_Inv_Mode != INV_SAVE_MODE) {
                if (g_PasswordText1 == NULL) {
                    g_PasswordText1 = Text_Create(
                        0, -16, g_GF_GameStrings[GF_S_GAME_PASSPORT_LOAD_GAME]);
                    Text_AlignBottom(g_PasswordText1, true);
                    Text_CentreH(g_PasswordText1, true);
                }

                Text_Remove(m_LevelText);
                m_LevelText = NULL;

                GetSavedGamesList(&g_LoadGameRequester);
                Requester_SetHeading(
                    &g_LoadGameRequester,
                    g_GF_GameStrings[GF_S_GAME_PASSPORT_LOAD_GAME], 0, NULL, 0);

                m_PassportMode = PASSPORT_MODE_LOAD_GAME;
                g_Input = (INPUT_STATE) {};
                g_InputDB = (INPUT_STATE) {};
            } else {
                g_InputDB = (INPUT_STATE) { .menu_right = 1 };
            }
        }
        break;

    case 1:
        if (g_GameFlow.load_save_disabled) {
            g_InputDB = (INPUT_STATE) { .menu_right = 1 };
            break;
        }

        if (m_PassportMode == PASSPORT_MODE_LOAD_GAME
            || m_PassportMode == PASSPORT_MODE_SELECT_LEVEL) {
            int32_t select;
            if (m_PassportMode == PASSPORT_MODE_LOAD_GAME) {
                Requester_SetSize(&g_LoadGameRequester, 10, -32);
                select = Requester_Display(&g_LoadGameRequester, 1, 1);
            } else {
                Requester_SetSize(&g_SaveGameRequester, 10, -32);
                select = Requester_Display(&g_SaveGameRequester, 1, 1);
            }

            if (select != 0) {
                if (select > 0) {
                    g_Inv_ExtraData[1] = select - 1;
                }
                m_PassportMode = PASSPORT_MODE_BROWSE;
            } else if (g_InputDB.menu_left || g_InputDB.menu_right) {
                if (m_PassportMode == PASSPORT_MODE_LOAD_GAME) {
                    Requester_Shutdown(&g_LoadGameRequester);
                } else {
                    Requester_Shutdown(&g_SaveGameRequester);
                }
                m_PassportMode = PASSPORT_MODE_BROWSE;
            } else {
                g_Input = (INPUT_STATE) {};
                g_InputDB = (INPUT_STATE) {};
            }
        } else if (m_PassportMode == PASSPORT_MODE_BROWSE) {
            if (g_Inv_Mode == INV_DEATH_MODE) {
                if (item->anim_direction == -1) {
                    g_InputDB = (INPUT_STATE) { .menu_left = 1 };
                } else {
                    g_InputDB = (INPUT_STATE) { .menu_right = 1 };
                }
                break;
            }

            if (g_PasswordText1 == NULL) {
                if (g_Inv_Mode == INV_TITLE_MODE || g_CurrentLevel == LV_GYM) {
                    g_PasswordText1 = Text_Create(
                        0, -16, g_GF_GameStrings[GF_S_GAME_PASSPORT_NEW_GAME]);
                } else {
                    g_PasswordText1 = Text_Create(
                        0, -16, g_GF_GameStrings[GF_S_GAME_PASSPORT_SAVE_GAME]);
                }
                Text_AlignBottom(g_PasswordText1, true);
                Text_CentreH(g_PasswordText1, true);
            }

            if (g_Inv_Mode != INV_TITLE_MODE && g_CurrentLevel != LV_GYM) {
                Text_Remove(m_LevelText);
                m_LevelText = NULL;

                GetSavedGamesList(&g_LoadGameRequester);
                Requester_SetHeading(
                    &g_LoadGameRequester,
                    g_GF_GameStrings[GF_S_GAME_PASSPORT_SAVE_GAME], 0, NULL, 0);

                m_PassportMode = PASSPORT_MODE_LOAD_GAME;
                g_Input = (INPUT_STATE) {};
                g_InputDB = (INPUT_STATE) {};
            } else if (g_GameFlow.play_any_level) {
                Text_Remove(m_LevelText);
                m_LevelText = NULL;

                Requester_Init(&g_SaveGameRequester);
                GetValidLevelsList(&g_SaveGameRequester);
                Requester_SetHeading(
                    &g_SaveGameRequester,
                    g_GF_GameStrings[GF_S_GAME_PASSPORT_SELECT_LEVEL], 0, NULL,
                    0);

                m_PassportMode = PASSPORT_MODE_SELECT_LEVEL;
                g_Input = (INPUT_STATE) {};
                g_InputDB = (INPUT_STATE) {};
            } else if (g_InputDB.menu_confirm) {
                g_Inv_ExtraData[1] = LV_FIRST;
            }
        }
        break;

    case 2:
        if (g_PasswordText1 == NULL) {
            if (g_Inv_Mode == INV_TITLE_MODE) {
                g_PasswordText1 = Text_Create(
                    0, -16, g_GF_GameStrings[GF_S_GAME_PASSPORT_EXIT_GAME]);
            } else if (g_GameFlow.demo_version) {
                g_PasswordText1 = Text_Create(
                    0, -16, g_GF_GameStrings[GF_S_GAME_PASSPORT_EXIT_DEMO]);
            } else {
                g_PasswordText1 = Text_Create(
                    0, -16, g_GF_GameStrings[GF_S_GAME_PASSPORT_EXIT_TO_TITLE]);
            }
            Text_AlignBottom(g_PasswordText1, true);
            Text_CentreH(g_PasswordText1, true);
        }
        break;
    }

    if (g_InputDB.menu_left
        && (g_Inv_Mode != INV_DEATH_MODE || g_SavedGames != 0)) {
        item->anim_direction = -1;
        item->goal_frame -= 5;

        if (g_SavedGames == 0) {
            if (item->goal_frame < item->open_frame + 5) {
                item->goal_frame = item->open_frame + 5;
            } else {
                Text_Remove(g_PasswordText1);
                g_PasswordText1 = NULL;
            }
        } else {
            if (item->goal_frame < item->open_frame) {
                item->goal_frame = item->open_frame;
            } else {
                Sound_Effect(SFX_MENU_PASSPORT, NULL, SPM_ALWAYS);
                Text_Remove(g_PasswordText1);
                g_PasswordText1 = NULL;
            }
        }
        g_Input = (INPUT_STATE) {};
        g_InputDB = (INPUT_STATE) {};
    }

    if (g_InputDB.menu_right) {
        item->anim_direction = 1;
        item->goal_frame += 5;

        if (item->goal_frame > item->frames_total - 6) {
            item->goal_frame = item->frames_total - 6;
        } else {
            Sound_Effect(SFX_MENU_PASSPORT, NULL, SPM_ALWAYS);
            Text_Remove(g_PasswordText1);
            g_PasswordText1 = NULL;
        }
        g_Input = (INPUT_STATE) {};
        g_InputDB = (INPUT_STATE) {};
    }

    if (g_InputDB.menu_back) {
        if (g_Inv_Mode == INV_DEATH_MODE) {
            g_Input = (INPUT_STATE) {};
            g_InputDB = (INPUT_STATE) {};
        } else {
            if (page == 2) {
                item->anim_direction = 1;
                item->goal_frame = item->frames_total - 1;
            } else {
                item->anim_direction = -1;
                item->goal_frame = 0;
            }
            Text_Remove(g_PasswordText1);
            g_PasswordText1 = NULL;
        }
    }

    if (g_InputDB.menu_confirm) {
        g_Inv_ExtraData[0] = page;
        if (page == 2) {
            item->anim_direction = 1;
            item->goal_frame = item->frames_total - 1;
        } else {
            item->anim_direction = -1;
            item->goal_frame = 0;
        }
        Text_Remove(g_PasswordText1);
        g_PasswordText1 = NULL;
    }
}

void Option_Passport_Draw(INVENTORY_ITEM *const item)
{
}

void Option_Passport_Shutdown(void)
{
    Text_Remove(m_LevelText);
    m_LevelText = NULL;
    Text_Remove(g_PasswordText1);
    g_PasswordText1 = NULL;

    Requester_Shutdown(&g_LoadGameRequester);
    Requester_Shutdown(&g_SaveGameRequester);

    m_PassportMode = PASSPORT_MODE_BROWSE;
}
