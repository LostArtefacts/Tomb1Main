#include "game/input.h"
#include "game/music.h"
#include "game/option/option.h"
#include "game/sound.h"
#include "game/text.h"
#include "global/funcs.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#include <stdio.h>

static void M_InitText(void);
static void M_ShutdownText(void);

static void M_InitText(void)
{
    CLAMPG(g_OptionMusicVolume, 10);
    CLAMPG(g_OptionSoundVolume, 10);

    char text[8];
    sprintf(text, "\\{icon music} %2d", g_OptionMusicVolume);
    g_SoundText[0] = Text_Create(0, 0, text);
    Text_AddBackground(g_SoundText[0], 128, 0, 0, 0, TS_REQUESTED);
    Text_AddOutline(g_SoundText[0], TS_REQUESTED);

    sprintf(text, "\\{icon sound} %2d", g_OptionSoundVolume);
    g_SoundText[1] = Text_Create(0, 25, text);

    g_SoundText[2] = Text_Create(0, -32, " ");
    Text_AddBackground(g_SoundText[2], 140, 85, 0, 0, TS_BACKGROUND);
    Text_AddOutline(g_SoundText[2], TS_BACKGROUND);

    g_SoundText[3] = Text_Create(0, -30, g_GF_PCStrings[GF_S_PC_SET_VOLUMES]);
    Text_AddBackground(g_SoundText[3], 136, 0, 0, 0, TS_HEADING);
    Text_AddOutline(g_SoundText[3], TS_HEADING);

    for (int32_t i = 0; i < 4; i++) {
        Text_CentreH(g_SoundText[i], true);
        Text_CentreV(g_SoundText[i], true);
    }
}

static void M_ShutdownText(void)
{
    for (int32_t i = 0; i < 4; i++) {
        Text_Remove(g_SoundText[i]);
        g_SoundText[i] = NULL;
    }
}

void Option_Sound_Shutdown(void)
{
    M_ShutdownText();
}

void __cdecl Option_Sound(INVENTORY_ITEM *const item)
{
    char text[8];

    if (g_SoundText[0] == NULL) {
        M_InitText();
    }

    if (g_InputDB.forward && g_SoundOptionLine > 0) {
        Text_RemoveOutline(g_SoundText[g_SoundOptionLine]);
        Text_RemoveBackground(g_SoundText[g_SoundOptionLine]);
        g_SoundOptionLine--;
        Text_AddBackground(
            g_SoundText[g_SoundOptionLine], 128, 0, 0, 0, TS_REQUESTED);
        Text_AddOutline(g_SoundText[g_SoundOptionLine], TS_REQUESTED);
    }

    if (g_InputDB.back && g_SoundOptionLine < 1) {
        Text_RemoveOutline(g_SoundText[g_SoundOptionLine]);
        Text_RemoveBackground(g_SoundText[g_SoundOptionLine]);
        g_SoundOptionLine++;
        Text_AddBackground(
            g_SoundText[g_SoundOptionLine], 128, 0, 0, 0, TS_REQUESTED);
        Text_AddOutline(g_SoundText[g_SoundOptionLine], TS_REQUESTED);
    }

    if (g_SoundOptionLine) {
        bool changed = false;
        if (g_Input.left && g_OptionSoundVolume > 0) {
            g_Inv_IsOptionsDelay = 1;
            g_Inv_OptionsDelayCounter = 10;
            g_OptionSoundVolume--;
            changed = true;
        } else if (g_Input.right && g_OptionSoundVolume < 10) {
            g_Inv_IsOptionsDelay = 1;
            g_Inv_OptionsDelayCounter = 10;
            g_OptionSoundVolume++;
            changed = true;
        }

        if (changed) {
            sprintf(text, "\\{icon sound} %2d", g_OptionSoundVolume);
            Text_ChangeText(g_SoundText[1], text);
            if (g_OptionSoundVolume) {
                Sound_SetMasterVolume(6 * g_OptionSoundVolume + 4);
            } else {
                Sound_SetMasterVolume(0);
            }

            Sound_Effect(SFX_MENU_PASSPORT, NULL, SPM_ALWAYS);
        }
    } else {
        bool changed = false;
        if (g_Input.left && g_OptionMusicVolume > 0) {
            g_OptionMusicVolume--;
            changed = true;
        } else if (g_Input.right && g_OptionMusicVolume < 10) {
            g_OptionMusicVolume++;
            changed = true;
        }

        if (changed) {
            g_Inv_IsOptionsDelay = 1;
            g_Inv_OptionsDelayCounter = 10;
            sprintf(text, "\\{icon music} %2d", g_OptionMusicVolume);
            Text_ChangeText(g_SoundText[0], text);

            if (g_OptionMusicVolume) {
                Music_SetVolume(25 * g_OptionMusicVolume + 5);
            } else {
                Music_SetVolume(0);
            }

            Sound_Effect(SFX_MENU_PASSPORT, NULL, SPM_ALWAYS);
        }
    }

    if (g_InputDB.menu_confirm || g_InputDB.menu_back) {
        Option_Sound_Shutdown();
    }
}
