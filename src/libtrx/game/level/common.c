#include "game/level/common.h"

#include "game/gamebuf.h"
#include "game/inject.h"
#include "game/rooms.h"

#include <assert.h>

void Level_ReadRoomMesh(const int32_t room_num, VFILE *const file)
{
    ROOM *const room = Room_Get(room_num);
    const INJECTION_MESH_META inj_data = Inject_GetRoomMeshMeta(room_num);

    const uint32_t mesh_length = VFile_ReadU32(file);
    size_t start_pos = VFile_GetPos(file);

    {
        // Temporarily retain raw data until parsing/output refactor complete.
        room->data =
            GameBuf_Alloc(sizeof(int16_t) * mesh_length, GBUF_ROOM_MESH);
        VFile_Read(file, room->data, sizeof(int16_t) * mesh_length);
        VFile_SetPos(file, start_pos);
    }

    {
        room->mesh.num_vertices = VFile_ReadS16(file);
        const int32_t alloc_count =
            room->mesh.num_vertices + inj_data.num_vertices;
        room->mesh.vertices =
            GameBuf_Alloc(sizeof(ROOM_VERTEX) * alloc_count, GBUF_ROOM_MESH);
        for (int32_t i = 0; i < room->mesh.num_vertices; i++) {
            ROOM_VERTEX *const vertex = &room->mesh.vertices[i];
            vertex->pos.x = VFile_ReadS16(file);
            vertex->pos.y = VFile_ReadS16(file);
            vertex->pos.z = VFile_ReadS16(file);
            vertex->shade = VFile_ReadU16(file);
            vertex->flags = 0;
        }
    }

    {
        room->mesh.num_face4s = VFile_ReadS16(file);
        const int32_t alloc_count = room->mesh.num_face4s + inj_data.num_quads;
        room->mesh.face4s =
            GameBuf_Alloc(sizeof(FACE4) * alloc_count, GBUF_ROOM_MESH);
        for (int32_t i = 0; i < room->mesh.num_face4s; i++) {
            FACE4 *const face = &room->mesh.face4s[i];
            for (int32_t j = 0; j < 4; j++) {
                face->vertices[j] = VFile_ReadU16(file);
            }
            face->texture = VFile_ReadU16(file);
        }
    }

    {
        room->mesh.num_face3s = VFile_ReadS16(file);
        const int32_t alloc_count =
            room->mesh.num_face3s + inj_data.num_triangles;
        room->mesh.face3s =
            GameBuf_Alloc(sizeof(FACE4) * alloc_count, GBUF_ROOM_MESH);
        for (int32_t i = 0; i < room->mesh.num_face3s; i++) {
            FACE3 *const face = &room->mesh.face3s[i];
            for (int32_t j = 0; j < 3; j++) {
                face->vertices[j] = VFile_ReadU16(file);
            }
            face->texture = VFile_ReadU16(file);
        }
    }

    {
        room->mesh.num_sprites = VFile_ReadS16(file);
        const int32_t alloc_count =
            room->mesh.num_sprites + inj_data.num_sprites;
        room->mesh.sprites =
            GameBuf_Alloc(sizeof(ROOM_SPRITE) * alloc_count, GBUF_ROOM_MESH);
        for (int32_t i = 0; i < room->mesh.num_sprites; i++) {
            ROOM_SPRITE *const sprite = &room->mesh.sprites[i];
            sprite->vertex = VFile_ReadU16(file);
            sprite->texture = VFile_ReadU16(file);
        }
    }

    const size_t total_read =
        (VFile_GetPos(file) - start_pos) / sizeof(int16_t);
    assert(total_read == mesh_length);
}
