#pragma once

#include "json.h"

void Config_LoadFromJSON(JSON_OBJECT *root_obj);
void Config_DumpToJSON(JSON_OBJECT *root_obj);
void Config_Sanitize(void);
