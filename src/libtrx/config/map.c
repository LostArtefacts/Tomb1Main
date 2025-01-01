#include "config/map.h"

#include "config/types.h"

const CONFIG_OPTION g_ConfigOptionMap[] = {
#include "map.def"
    {}, // sentinel
};
