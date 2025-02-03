#include "game/objects/common.h"

#include "game/anims.h"
#include "game/const.h"
#include "game/game_buf.h"

static OBJECT m_Objects[O_NUMBER_OF] = {};
static STATIC_OBJECT_3D m_StaticObjects3D[MAX_STATIC_OBJECTS] = {};
static STATIC_OBJECT_2D m_StaticObjects2D[MAX_STATIC_OBJECTS] = {};
static OBJECT_MESH **m_MeshPointers = nullptr;
static int32_t m_MeshCount = 0;

OBJECT *Object_Get(const GAME_OBJECT_ID object_id)
{
    return &m_Objects[object_id];
}

STATIC_OBJECT_3D *Object_Get3DStatic(const int32_t static_id)
{
    return &m_StaticObjects3D[static_id];
}

STATIC_OBJECT_2D *Object_Get2DStatic(const int32_t static_id)
{
    return &m_StaticObjects2D[static_id];
}

bool Object_IsObjectType(
    GAME_OBJECT_ID object_id, const GAME_OBJECT_ID *test_arr)
{
    for (int i = 0; test_arr[i] != NO_OBJECT; i++) {
        if (test_arr[i] == object_id) {
            return true;
        }
    }
    return false;
}

GAME_OBJECT_ID Object_GetCognate(
    GAME_OBJECT_ID key_id, const GAME_OBJECT_PAIR *test_map)
{
    const GAME_OBJECT_PAIR *pair = &test_map[0];
    while (pair->key_id != NO_OBJECT) {
        if (pair->key_id == key_id) {
            return pair->value_id;
        }
        pair++;
    }

    return NO_OBJECT;
}

GAME_OBJECT_ID Object_GetCognateInverse(
    GAME_OBJECT_ID value_id, const GAME_OBJECT_PAIR *test_map)
{
    const GAME_OBJECT_PAIR *pair = &test_map[0];
    while (pair->key_id != NO_OBJECT) {
        if (pair->value_id == value_id) {
            return pair->key_id;
        }
        pair++;
    }

    return NO_OBJECT;
}

void Object_InitialiseMeshes(const int32_t mesh_count)
{
    m_MeshPointers =
        GameBuf_Alloc(sizeof(OBJECT_MESH *) * mesh_count, GBUF_MESH_POINTERS);
    m_MeshCount = 0;
}

void Object_StoreMesh(OBJECT_MESH *const mesh)
{
    m_MeshPointers[m_MeshCount] = mesh;
    m_MeshCount++;
}

OBJECT_MESH *Object_GetMesh(const int32_t index)
{
    return m_MeshPointers[index];
}

OBJECT_MESH *Object_FindMesh(const int32_t data_offset)
{
    for (int32_t i = 0; i < m_MeshCount; i++) {
        OBJECT_MESH *const mesh = m_MeshPointers[i];
        if (Object_GetMeshOffset(mesh) == data_offset) {
            return mesh;
        }
    }

    return nullptr;
}

int32_t Object_GetMeshOffset(const OBJECT_MESH *const mesh)
{
    return (int32_t)(intptr_t)mesh->priv;
}

void Object_SetMeshOffset(OBJECT_MESH *const mesh, const int32_t data_offset)
{
    mesh->priv = (void *)(intptr_t)data_offset;
}

void Object_SwapMesh(
    const GAME_OBJECT_ID object1_id, const GAME_OBJECT_ID object2_id,
    const int32_t mesh_num)
{
    const OBJECT *const obj1 = Object_Get(object1_id);
    const OBJECT *const obj2 = Object_Get(object2_id);

    OBJECT_MESH *const temp = m_MeshPointers[obj1->mesh_idx + mesh_num];
    m_MeshPointers[obj1->mesh_idx + mesh_num] =
        m_MeshPointers[obj2->mesh_idx + mesh_num];
    m_MeshPointers[obj2->mesh_idx + mesh_num] = temp;
}

ANIM *Object_GetAnim(const OBJECT *const obj, const int32_t anim_idx)
{
    return Anim_GetAnim(obj->anim_idx + anim_idx);
}

ANIM_BONE *Object_GetBone(const OBJECT *const obj, const int32_t bone_idx)
{
    return Anim_GetBone(obj->bone_idx + bone_idx);
}
