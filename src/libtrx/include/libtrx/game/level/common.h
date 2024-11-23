#pragma once

#include "../../virtual_file.h"

#if TR_VERSION == 1
void Level_ReadRoomMesh(int32_t room_num, VFILE *file);
void Level_ReadObjectMeshes(
    int32_t num_indices, const int32_t *indices, VFILE *file);
#endif
