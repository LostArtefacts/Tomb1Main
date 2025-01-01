#include "config/common.h"

#include "config/file.h"
#include "config/map.h"
#include "config/priv.h"
#include "debug.h"
#include "game/shell.h"

typedef enum {
    CFT_DEFAULT,
    CFT_ENFORCED,
} CONFIG_FILE_TYPE;

EVENT_MANAGER *m_EventManager = NULL;

static const char *M_GetPath(CONFIG_FILE_TYPE file_type);

static const char *M_GetPath(const CONFIG_FILE_TYPE file_type)
{
    return file_type == CFT_DEFAULT ? Shell_GetConfigPath()
                                    : Shell_GetGameflowPath();
}

void Config_Init(void)
{
    m_EventManager = EventManager_Create();
}

void Config_Shutdown(void)
{
    EventManager_Free(m_EventManager);
    m_EventManager = NULL;
}

bool Config_Read(void)
{
    const CONFIG_IO_ARGS args = {
        .default_path = M_GetPath(CFT_DEFAULT),
        .enforced_path = M_GetPath(CFT_ENFORCED),
        .action = &Config_LoadFromJSON,
    };
    const bool result = ConfigFile_Read(&args);
    if (result) {
        Config_Sanitize();
        g_SavedConfig = g_Config;
    }
    return result;
}

bool Config_Write(void)
{
    Config_Sanitize();
    const CONFIG_IO_ARGS args = {
        .default_path = M_GetPath(CFT_DEFAULT),
        .enforced_path = M_GetPath(CFT_ENFORCED),
        .action = &Config_DumpToJSON,
    };
    const bool updated = ConfigFile_Write(&args);
    if (updated) {
        if (m_EventManager != NULL) {
            const EVENT event = {
                .name = "change",
                .sender = NULL,
                .data = NULL,
            };
            EventManager_Fire(m_EventManager, &event);
        }
        g_SavedConfig = g_Config;
    }
    return updated;
}

int32_t Config_SubscribeChanges(
    const EVENT_LISTENER listener, void *const user_data)
{
    ASSERT(m_EventManager != NULL);
    return EventManager_Subscribe(
        m_EventManager, "change", NULL, listener, user_data);
}

void Config_UnsubscribeChanges(const int32_t listener_id)
{
    ASSERT(m_EventManager != NULL);
    return EventManager_Unsubscribe(m_EventManager, listener_id);
}

const CONFIG_OPTION *Config_GetOptionMap(void)
{
    return g_ConfigOptionMap;
}
