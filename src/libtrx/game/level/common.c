#include "game/level/common.h"

#include "game/gamebuf.h"
#include "game/inject.h"
#include "game/rooms.h"

void Level_ReadRoomMesh(const int32_t room_num, VFILE *const file)
{
    ROOM *const room = Room_Get(room_num);
    const INJECTION_MESH_META inj_data = Inject_GetRoomMeshMeta(room_num);

    const uint32_t mesh_length = VFile_ReadU32(file);
    room->data = GameBuf_Alloc(sizeof(int16_t) * mesh_length, GBUF_ROOM_MESH);
    VFile_Read(file, room->data, sizeof(int16_t) * mesh_length);
}
