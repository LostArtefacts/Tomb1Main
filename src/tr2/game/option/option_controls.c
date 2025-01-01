#include "game/option/option.h"
#include "game/ui/widgets/controls_dialog.h"
#include "global/vars.h"

#include <libtrx/config.h>
#include <libtrx/game/ui/events.h>

static UI_WIDGET *m_Dialog;
static UI_CONTROLS_CONTROLLER m_Controller;
static int32_t m_Listener1;
static int32_t m_Listener2;

static void M_Init(void);
static void M_Shutdown(void);
static void M_HandleLayoutChange(const EVENT *event, void *user_data);
static void M_HandleKeyChange(const EVENT *event, void *user_data);

static void M_Init(void)
{
    UI_ControlsController_Init(&m_Controller);
    m_Controller.active_layout = g_Config.input.keyboard_layout;

    m_Dialog = UI_ControlsDialog_Create(&m_Controller);
    m_Listener1 = EventManager_Subscribe(
        m_Controller.events, "layout_change", NULL, M_HandleLayoutChange, NULL);
    m_Listener2 = EventManager_Subscribe(
        m_Controller.events, "key_change", NULL, M_HandleKeyChange, NULL);
}

static void M_Shutdown(void)
{
    if (m_Dialog == NULL) {
        return;
    }

    m_Dialog->free(m_Dialog);
    m_Dialog = NULL;

    EventManager_Unsubscribe(m_Controller.events, m_Listener1);
    EventManager_Unsubscribe(m_Controller.events, m_Listener2);

    UI_ControlsController_Shutdown(&m_Controller);
}

static void M_HandleLayoutChange(const EVENT *event, void *user_data)
{
    switch (m_Controller.backend) {
    case INPUT_BACKEND_KEYBOARD:
        g_Config.input.keyboard_layout = m_Controller.active_layout;
        break;
    case INPUT_BACKEND_CONTROLLER:
        g_Config.input.controller_layout = m_Controller.active_layout;
        break;
    default:
        break;
    }

    Config_Write();
}

static void M_HandleKeyChange(const EVENT *event, void *user_data)
{
    Config_Write();
}

void Option_Controls_Shutdown(void)
{
    M_Shutdown();
}

void Option_Controls_Control(INV_ITEM *const item)
{
    if (m_Dialog == NULL) {
        M_Init();
    }

    m_Dialog->control(m_Dialog);
    if (m_Controller.state == UI_CONTROLS_STATE_EXIT) {
        Option_Controls_Shutdown();
    } else {
        g_Input = (INPUT_STATE) { 0 };
        g_InputDB = (INPUT_STATE) { 0 };
    }
}

void Option_Controls_Draw(INV_ITEM *const item)
{
    if (m_Dialog != NULL) {
        m_Dialog->draw(m_Dialog);
    }
}
