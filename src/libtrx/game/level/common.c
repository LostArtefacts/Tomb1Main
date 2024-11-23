#include "game/level/common.h"

#include "game/gamebuf.h"
#include "game/inject.h"
#include "game/objects/common.h"
#include "game/rooms.h"
#include "log.h"
#include "utils.h"
#include "vector.h"

#include <assert.h>

#if TR_VERSION == 1
// TODO: create shared readers for XYZ_16 and faces
static void M_ReadObjectMesh(OBJECT_MESH *mesh, VFILE *file);

static void M_ReadObjectMesh(OBJECT_MESH *const mesh, VFILE *const file)
{
    mesh->center.x = VFile_ReadS16(file);
    mesh->center.y = VFile_ReadS16(file);
    mesh->center.z = VFile_ReadS16(file);
    mesh->radius = VFile_ReadS32(file);

    mesh->enable_reflections = false;

    {
        mesh->num_vertices = VFile_ReadS16(file);
        mesh->vertices =
            GameBuf_Alloc(sizeof(XYZ_16) * mesh->num_vertices, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_vertices; i++) {
            XYZ_16 *const vertex = &mesh->vertices[i];
            vertex->x = VFile_ReadS16(file);
            vertex->y = VFile_ReadS16(file);
            vertex->z = VFile_ReadS16(file);
        }
    }

    {
        mesh->num_lights = VFile_ReadS16(file);
        if (mesh->num_lights > 0) {
            mesh->lighting.normals =
                GameBuf_Alloc(sizeof(XYZ_16) * mesh->num_lights, GBUF_MESHES);
            for (int32_t i = 0; i < mesh->num_lights; i++) {
                XYZ_16 *const normal = &mesh->lighting.normals[i];
                normal->x = VFile_ReadS16(file);
                normal->y = VFile_ReadS16(file);
                normal->z = VFile_ReadS16(file);
            }
        } else {
            mesh->lighting.lights = GameBuf_Alloc(
                sizeof(int16_t) * ABS(mesh->num_lights), GBUF_MESHES);
            for (int32_t i = 0; i < ABS(mesh->num_lights); i++) {
                mesh->lighting.lights[i] = VFile_ReadS16(file);
            }
        }
    }

    {
        mesh->num_tex_face4s = VFile_ReadS16(file);
        mesh->tex_face4s =
            GameBuf_Alloc(sizeof(FACE4) * mesh->num_tex_face4s, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_tex_face4s; i++) {
            FACE4 *const face = &mesh->tex_face4s[i];
            for (int32_t j = 0; j < 4; j++) {
                face->vertices[j] = VFile_ReadU16(file);
            }
            face->texture = VFile_ReadU16(file);
            face->enable_reflections = false;
        }
    }

    {
        mesh->num_tex_face3s = VFile_ReadS16(file);
        mesh->tex_face3s =
            GameBuf_Alloc(sizeof(FACE3) * mesh->num_tex_face3s, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_tex_face3s; i++) {
            FACE3 *const face = &mesh->tex_face3s[i];
            for (int32_t j = 0; j < 3; j++) {
                face->vertices[j] = VFile_ReadU16(file);
            }
            face->texture = VFile_ReadU16(file);
            face->enable_reflections = false;
        }
    }

    {
        mesh->num_flat_face4s = VFile_ReadS16(file);
        mesh->flat_face4s =
            GameBuf_Alloc(sizeof(FACE4) * mesh->num_flat_face4s, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_flat_face4s; i++) {
            FACE4 *const face = &mesh->flat_face4s[i];
            for (int32_t j = 0; j < 4; j++) {
                face->vertices[j] = VFile_ReadU16(file);
            }
            face->texture = VFile_ReadU16(file);
            face->enable_reflections = false;
        }
    }

    {
        mesh->num_flat_face3s = VFile_ReadS16(file);
        mesh->flat_face3s =
            GameBuf_Alloc(sizeof(FACE3) * mesh->num_flat_face3s, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_flat_face3s; i++) {
            FACE3 *const face = &mesh->flat_face3s[i];
            for (int32_t j = 0; j < 3; j++) {
                face->vertices[j] = VFile_ReadU16(file);
            }
            face->texture = VFile_ReadU16(file);
            face->enable_reflections = false;
        }
    }
}

void Level_ReadRoomMesh(const int32_t room_num, VFILE *const file)
{
    ROOM *const room = Room_Get(room_num);
    const INJECTION_MESH_META inj_data = Inject_GetRoomMeshMeta(room_num);

    const uint32_t mesh_length = VFile_ReadU32(file);
    size_t start_pos = VFile_GetPos(file);

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

void Level_ReadObjectMeshes(
    const int32_t num_indices, const int32_t *const indices, VFILE *const file)
{
    // Construct and store distinct meshes only e.g. Lara's hips are referenced
    // by several pointers as a dummy mesh.
    VECTOR *const unique_indices =
        Vector_CreateAtCapacity(sizeof(int32_t), num_indices);
    int32_t pointer_map[num_indices];
    for (int32_t i = 0; i < num_indices; i++) {
        const int32_t pointer = indices[i];
        const int32_t index = Vector_IndexOf(unique_indices, (void *)&pointer);
        if (index == -1) {
            pointer_map[i] = unique_indices->count;
            Vector_Add(unique_indices, (void *)&pointer);
        } else {
            pointer_map[i] = index;
        }
    }

    OBJECT_MESH *const meshes =
        GameBuf_Alloc(sizeof(OBJECT_MESH) * unique_indices->count, GBUF_MESHES);
    size_t start_pos = VFile_GetPos(file);
    for (int i = 0; i < unique_indices->count; i++) {
        const int32_t pointer = *(const int32_t *)Vector_Get(unique_indices, i);
        VFile_SetPos(file, start_pos + pointer);
        M_ReadObjectMesh(&meshes[i], file);

        // The original data position is required for backward compatibility
        // with savegame files, specifically for Lara's mesh pointers.
        Object_SetMeshOffset(&meshes[i], pointer / 2);
    }

    for (int32_t i = 0; i < num_indices; i++) {
        Object_StoreMesh(&meshes[pointer_map[i]]);
    }

    LOG_INFO("%d unique meshes constructed", unique_indices->count);

    Vector_Free(unique_indices);
}
#endif
