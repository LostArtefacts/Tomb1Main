#include "config/option.h"
#include "config/types.h"
#include "config/vars.h"
#include "enum_map.h"
#include "utils.h"

#define CFG_BOOL(parent, target_, default_value_)                              \
    { .name = QUOTE(target_),                                                  \
      .type = COT_BOOL,                                                        \
      .target = &parent.target_,                                               \
      .default_value = &(bool) { default_value_ },                             \
      .param = nullptr },

#define CFG_INT32(parent, target_, default_value_)                             \
    { .name = QUOTE(target_),                                                  \
      .type = COT_INT32,                                                       \
      .target = &parent.target_,                                               \
      .default_value = &(int32_t) { default_value_ },                          \
      .param = nullptr },

#define CFG_FLOAT(parent, target_, default_value_)                             \
    { .name = QUOTE(target_),                                                  \
      .type = COT_FLOAT,                                                       \
      .target = &parent.target_,                                               \
      .default_value = &(float) { default_value_ },                            \
      .param = nullptr },

#define CFG_DOUBLE(parent, target_, default_value_)                            \
    { .name = QUOTE(target_),                                                  \
      .type = COT_DOUBLE,                                                      \
      .target = &parent.target_,                                               \
      .default_value = &(double) { default_value_ },                           \
      .param = nullptr },

#define CFG_ENUM(parent, target_, default_value_, enum_map)                    \
    { .name = QUOTE(target_),                                                  \
      .type = COT_ENUM,                                                        \
      .target = &parent.target_,                                               \
      .default_value = &(int32_t) { default_value_ },                          \
      .param = ENUM_MAP_NAME(enum_map) },

static const CONFIG_OPTION m_ConfigOptionMap[] = {
#include "map.def"
    {}, // sentinel
};

const CONFIG_OPTION *Config_GetOptionMap(void)
{
    return m_ConfigOptionMap;
}
