#include "game/level/common.h"

#include "debug.h"
#include "game/anims.h"
#include "game/const.h"
#include "game/game_buf.h"
#include "game/inject.h"
#include "game/objects/common.h"
#include "game/output.h"
#include "game/rooms.h"
#include "game/shell.h"
#include "log.h"
#include "memory.h"
#include "utils.h"
#include "vector.h"

static int16_t *m_AnimCommands = NULL;

static void M_ReadVertex(XYZ_16 *vertex, VFILE *file);
static void M_ReadFace4(FACE4 *face, VFILE *file);
static void M_ReadFace3(FACE3 *face, VFILE *file);
static void M_ReadObjectMesh(OBJECT_MESH *mesh, VFILE *file);
static void M_ReadBounds16(BOUNDS_16 *bounds, VFILE *file);

static void M_ReadVertex(XYZ_16 *const vertex, VFILE *const file)
{
    vertex->x = VFile_ReadS16(file);
    vertex->y = VFile_ReadS16(file);
    vertex->z = VFile_ReadS16(file);
}

static void M_ReadFace4(FACE4 *const face, VFILE *const file)
{
    for (int32_t i = 0; i < 4; i++) {
        face->vertices[i] = VFile_ReadU16(file);
    }
    face->texture_idx = VFile_ReadU16(file);
    face->enable_reflections = false;
}

static void M_ReadFace3(FACE3 *const face, VFILE *const file)
{
    for (int32_t i = 0; i < 3; i++) {
        face->vertices[i] = VFile_ReadU16(file);
    }
    face->texture_idx = VFile_ReadU16(file);
    face->enable_reflections = false;
}

static void M_ReadObjectMesh(OBJECT_MESH *const mesh, VFILE *const file)
{
    M_ReadVertex(&mesh->center, file);
    mesh->radius = VFile_ReadS16(file);
    VFile_Skip(file, sizeof(int16_t));

    mesh->enable_reflections = false;

    {
        mesh->num_vertices = VFile_ReadS16(file);
        mesh->vertices =
            GameBuf_Alloc(sizeof(XYZ_16) * mesh->num_vertices, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_vertices; i++) {
            M_ReadVertex(&mesh->vertices[i], file);
        }
    }

    {
        mesh->num_lights = VFile_ReadS16(file);
        if (mesh->num_lights > 0) {
            mesh->lighting.normals =
                GameBuf_Alloc(sizeof(XYZ_16) * mesh->num_lights, GBUF_MESHES);
            for (int32_t i = 0; i < mesh->num_lights; i++) {
                M_ReadVertex(&mesh->lighting.normals[i], file);
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
            M_ReadFace4(&mesh->tex_face4s[i], file);
        }
    }

    {
        mesh->num_tex_face3s = VFile_ReadS16(file);
        mesh->tex_face3s =
            GameBuf_Alloc(sizeof(FACE3) * mesh->num_tex_face3s, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_tex_face3s; i++) {
            M_ReadFace3(&mesh->tex_face3s[i], file);
        }
    }

    {
        mesh->num_flat_face4s = VFile_ReadS16(file);
        mesh->flat_face4s =
            GameBuf_Alloc(sizeof(FACE4) * mesh->num_flat_face4s, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_flat_face4s; i++) {
            M_ReadFace4(&mesh->flat_face4s[i], file);
        }
    }

    {
        mesh->num_flat_face3s = VFile_ReadS16(file);
        mesh->flat_face3s =
            GameBuf_Alloc(sizeof(FACE3) * mesh->num_flat_face3s, GBUF_MESHES);
        for (int32_t i = 0; i < mesh->num_flat_face3s; i++) {
            M_ReadFace3(&mesh->flat_face3s[i], file);
        }
    }
}

static void M_ReadBounds16(BOUNDS_16 *const bounds, VFILE *const file)
{
    bounds->min.x = VFile_ReadS16(file);
    bounds->max.x = VFile_ReadS16(file);
    bounds->min.y = VFile_ReadS16(file);
    bounds->max.y = VFile_ReadS16(file);
    bounds->min.z = VFile_ReadS16(file);
    bounds->max.z = VFile_ReadS16(file);
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
            M_ReadVertex(&vertex->pos, file);
            vertex->light_base = VFile_ReadS16(file);
#if TR_VERSION == 1
            vertex->flags = 0;
            vertex->light_adder = vertex->light_base;
#elif TR_VERSION == 2
            vertex->light_table_value = VFile_ReadU8(file);
            vertex->flags = VFile_ReadU8(file);
            vertex->light_adder = VFile_ReadS16(file);
#endif
        }
    }

    {
        room->mesh.num_face4s = VFile_ReadS16(file);
        const int32_t alloc_count = room->mesh.num_face4s + inj_data.num_quads;
        room->mesh.face4s =
            GameBuf_Alloc(sizeof(FACE4) * alloc_count, GBUF_ROOM_MESH);
        for (int32_t i = 0; i < room->mesh.num_face4s; i++) {
            M_ReadFace4(&room->mesh.face4s[i], file);
        }
    }

    {
        room->mesh.num_face3s = VFile_ReadS16(file);
        const int32_t alloc_count =
            room->mesh.num_face3s + inj_data.num_triangles;
        room->mesh.face3s =
            GameBuf_Alloc(sizeof(FACE4) * alloc_count, GBUF_ROOM_MESH);
        for (int32_t i = 0; i < room->mesh.num_face3s; i++) {
            M_ReadFace3(&room->mesh.face3s[i], file);
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
    ASSERT(total_read == mesh_length);
}

void Level_ReadFloorData(VFILE *const file)
{
    const int32_t floor_data_size = VFile_ReadS32(file);
    int16_t *floor_data = Memory_Alloc(sizeof(int16_t) * floor_data_size);
    VFile_Read(file, floor_data, sizeof(int16_t) * floor_data_size);

    Room_ParseFloorData(floor_data);
    Memory_FreePointer(&floor_data);
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

void Level_ReadAnims(
    const int32_t base_idx, const int32_t num_anims, VFILE *const file)
{
    for (int32_t i = 0; i < num_anims; i++) {
        ANIM *const anim = Anim_GetAnim(base_idx + i);
        anim->frame_ofs = VFile_ReadU32(file);
        anim->frame_ptr = NULL; // filled later by the animation frame loader
        anim->interpolation = VFile_ReadU8(file);
        anim->frame_size = VFile_ReadU8(file);
        anim->current_anim_state = VFile_ReadS16(file);
        anim->velocity = VFile_ReadS32(file);
        anim->acceleration = VFile_ReadS32(file);
        anim->frame_base = VFile_ReadS16(file);
        anim->frame_end = VFile_ReadS16(file);
        anim->jump_anim_num = VFile_ReadS16(file);
        anim->jump_frame_num = VFile_ReadS16(file);
        anim->num_changes = VFile_ReadS16(file);
        anim->change_idx = VFile_ReadS16(file);
        anim->num_commands = VFile_ReadS16(file);
        anim->command_idx = VFile_ReadS16(file);
    }
}

void Level_ReadAnimChanges(
    const int32_t base_idx, const int32_t num_changes, VFILE *const file)
{
    for (int32_t i = 0; i < num_changes; i++) {
        ANIM_CHANGE *const anim_change = Anim_GetChange(base_idx + i);
        anim_change->goal_anim_state = VFile_ReadS16(file);
        anim_change->num_ranges = VFile_ReadS16(file);
        anim_change->range_idx = VFile_ReadS16(file);
    }
}

void Level_ReadAnimRanges(
    const int32_t base_idx, const int32_t num_ranges, VFILE *const file)
{
    for (int32_t i = 0; i < num_ranges; i++) {
        ANIM_RANGE *const anim_range = Anim_GetRange(base_idx + i);
        anim_range->start_frame = VFile_ReadS16(file);
        anim_range->end_frame = VFile_ReadS16(file);
        anim_range->link_anim_num = VFile_ReadS16(file);
        anim_range->link_frame_num = VFile_ReadS16(file);
    }
}

void Level_InitialiseAnimCommands(const int32_t num_cmds)
{
    m_AnimCommands = Memory_Alloc(sizeof(int16_t) * num_cmds);
}

void Level_ReadAnimCommands(
    const int32_t base_idx, const int32_t num_cmds, VFILE *const file)
{
    VFile_Read(file, m_AnimCommands + base_idx, sizeof(int16_t) * num_cmds);
}

void Level_LoadAnimCommands(void)
{
    Anim_LoadCommands(m_AnimCommands);
    Memory_FreePointer(&m_AnimCommands);
}

void Level_ReadAnimBones(
    const int32_t base_idx, const int32_t num_bones, VFILE *const file)
{
    for (int32_t i = 0; i < num_bones; i++) {
        ANIM_BONE *const bone = Anim_GetBone(base_idx + i);
        const int32_t flags = VFile_ReadS32(file);
        bone->matrix_pop = (flags & 1) != 0;
        bone->matrix_push = (flags & 2) != 0;
        bone->rot_x = false;
        bone->rot_y = false;
        bone->rot_z = false;
        bone->pos.x = VFile_ReadS32(file);
        bone->pos.y = VFile_ReadS32(file);
        bone->pos.z = VFile_ReadS32(file);
    }
}

void Level_ReadObjects(const int32_t num_objects, VFILE *const file)
{
    for (int32_t i = 0; i < num_objects; i++) {
        const GAME_OBJECT_ID object_id = VFile_ReadS32(file);
        if (object_id < 0 || object_id >= O_NUMBER_OF) {
            Shell_ExitSystemFmt(
                "Invalid object ID: %d (max=%d)", object_id, O_NUMBER_OF);
        }

        OBJECT *const object = Object_GetObject(object_id);
        object->mesh_count = VFile_ReadS16(file);
        object->mesh_idx = VFile_ReadS16(file);
        object->bone_idx = VFile_ReadS32(file) / ANIM_BONE_SIZE;
        object->frame_ofs = VFile_ReadU32(file);
        object->frame_base = NULL;
        object->anim_idx = VFile_ReadS16(file);
        object->loaded = true;
    }
}

void Level_ReadStaticObjects(const int32_t num_objects, VFILE *const file)
{
    for (int32_t i = 0; i < num_objects; i++) {
        const int32_t static_id = VFile_ReadS32(file);
        if (static_id < 0 || static_id >= MAX_STATIC_OBJECTS) {
            Shell_ExitSystemFmt(
                "Invalid static ID: %d (max=%d)", static_id,
                MAX_STATIC_OBJECTS);
        }

        STATIC_OBJECT_3D *const static_obj =
            Object_GetStaticObject3D(static_id);
        static_obj->mesh_idx = VFile_ReadS16(file);
        static_obj->loaded = true;

        M_ReadBounds16(&static_obj->draw_bounds, file);
        M_ReadBounds16(&static_obj->collision_bounds, file);

        const uint16_t flags = VFile_ReadU16(file);
        static_obj->collidable = (flags & 1) == 0;
        static_obj->visible = (flags & 2) != 0;
    }
}

void Level_ReadObjectTextures(
    const int32_t base_idx, const int16_t base_page_idx,
    const int32_t num_textures, VFILE *const file)
{
    for (int32_t i = 0; i < num_textures; i++) {
        OBJECT_TEXTURE *const texture = Output_GetObjectTexture(base_idx + i);
        texture->draw_type = VFile_ReadU16(file);
        texture->tex_page = VFile_ReadU16(file) + base_page_idx;
        for (int32_t j = 0; j < 4; j++) {
            texture->uv[j].u = VFile_ReadU16(file);
            texture->uv[j].v = VFile_ReadU16(file);
        }
    }
}

void Level_ReadSpriteTextures(
    const int32_t base_idx, const int16_t base_page_idx,
    const int32_t num_textures, VFILE *const file)
{
    for (int32_t i = 0; i < num_textures; i++) {
        SPRITE_TEXTURE *const sprite = Output_GetSpriteTexture(base_idx + i);
        sprite->tex_page = VFile_ReadU16(file) + base_page_idx;
        sprite->offset = VFile_ReadU16(file);
        sprite->width = VFile_ReadU16(file);
        sprite->height = VFile_ReadU16(file);
        sprite->x0 = VFile_ReadS16(file);
        sprite->y0 = VFile_ReadS16(file);
        sprite->x1 = VFile_ReadS16(file);
        sprite->y1 = VFile_ReadS16(file);
    }
}

void Level_ReadSpriteSequences(const int32_t num_sequences, VFILE *const file)
{
    for (int32_t i = 0; i < num_sequences; i++) {
        const int32_t object_id = VFile_ReadS32(file);
        const int16_t num_meshes = VFile_ReadS16(file);
        const int16_t mesh_idx = VFile_ReadS16(file);

        if (object_id >= 0 && object_id < O_NUMBER_OF) {
            OBJECT *const object = Object_GetObject(object_id);
            object->mesh_count = num_meshes;
            object->mesh_idx = mesh_idx;
            object->loaded = true;
        } else if (object_id - O_NUMBER_OF < MAX_STATIC_OBJECTS) {
            STATIC_OBJECT_2D *const object =
                Object_GetStaticObject2D(object_id - O_NUMBER_OF);
            object->frame_count = ABS(num_meshes);
            object->texture_idx = mesh_idx;
            object->loaded = true;
        } else {
            Shell_ExitSystemFmt("Invalid sprite slot (%d)", object_id);
        }
    }
}

void Level_ReadAnimatedTextureRanges(
    const int32_t num_ranges, VFILE *const file)
{
    for (int32_t i = 0; i < num_ranges; i++) {
        ANIMATED_TEXTURE_RANGE *const range = Output_GetAnimatedTextureRange(i);
        range->next_range =
            i == num_ranges - 1 ? NULL : Output_GetAnimatedTextureRange(i + 1);

        // Level data is tied to the original logic in Output_AnimateTextures
        // and hence stores one less than the actual count here.
        range->num_textures = VFile_ReadS16(file) + 1;
        range->textures = GameBuf_Alloc(
            sizeof(int16_t) * range->num_textures,
            GBUF_ANIMATED_TEXTURE_RANGES);
        VFile_Read(
            file, range->textures, sizeof(int16_t) * range->num_textures);
    }
}
