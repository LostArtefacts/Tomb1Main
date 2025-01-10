#include "game/level.h"

#include "decomp/decomp.h"
#include "game/effects.h"
#include "game/gameflow/gameflow_new.h"
#include "game/inject.h"
#include "game/items.h"
#include "game/objects/setup.h"
#include "game/output.h"
#include "game/render/common.h"
#include "game/room.h"
#include "game/shell.h"
#include "game/sound.h"
#include "global/const.h"
#include "global/vars.h"

#include <libtrx/benchmark.h>
#include <libtrx/debug.h>
#include <libtrx/engine/audio.h>
#include <libtrx/filesystem.h>
#include <libtrx/game/gamebuf.h>
#include <libtrx/game/level.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>

static int16_t *m_FloorData = NULL;
static int16_t *m_AnimFrameData = NULL;
static int32_t m_AnimFrameDataLength = 0;

static void M_LoadFromFile(const char *file_name, int32_t level_num);
static void M_LoadRooms(VFILE *file);
static void M_LoadMeshBase(VFILE *file);
static void M_LoadMeshes(VFILE *file);
static void M_LoadAnims(VFILE *file);
static void M_LoadAnimChanges(VFILE *file);
static void M_LoadAnimRanges(VFILE *file);
static void M_LoadAnimCommands(VFILE *file);
static void M_LoadAnimBones(VFILE *file);
static void M_LoadAnimFrames(VFILE *file);
static void M_LoadObjects(VFILE *file);
static void M_LoadStaticObjects(VFILE *file);
static void M_LoadTextures(VFILE *file);
static void M_LoadSprites(VFILE *file);
static void M_LoadItems(VFILE *file);
static void M_LoadCameras(VFILE *file);
static void M_LoadSoundEffects(VFILE *file);
static void M_LoadBoxes(VFILE *file);
static void M_LoadAnimatedTextures(VFILE *file);
static void M_LoadCinematic(VFILE *file);
static void M_LoadDemo(VFILE *file);
static void M_LoadSamples(VFILE *file);
static void M_CompleteSetup(void);

static void M_LoadTexturePages(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    const int32_t texture_size = TEXTURE_PAGE_WIDTH * TEXTURE_PAGE_HEIGHT;
    const int32_t texture_size_8_bit = texture_size * sizeof(uint8_t);
    const int32_t texture_size_16_bit = texture_size * sizeof(uint16_t);

    const int32_t num_pages = VFile_ReadS32(file);
    LOG_INFO("texture pages: %d", num_pages);

    for (int32_t i = 0; i < num_pages; i++) {
        if (g_TexturePageBuffer8[i] == NULL) {
            g_TexturePageBuffer8[i] =
                GameBuf_Alloc(texture_size_8_bit, GBUF_TEXTURE_PAGES);
        }
        VFile_Read(file, g_TexturePageBuffer8[i], texture_size_8_bit);
    }
    for (int32_t i = 0; i < num_pages; i++) {
        if (g_TexturePageBuffer16[i] == NULL) {
            g_TexturePageBuffer16[i] =
                GameBuf_Alloc(texture_size_16_bit, GBUF_TEXTURE_PAGES);
        }
        VFile_Read(file, g_TexturePageBuffer16[i], texture_size_16_bit);
        LOG_DEBUG("%d", g_TexturePageBuffer16[0][0]);
    }

    g_TexturePageCount = num_pages;

finish:
    Benchmark_End(benchmark, NULL);
}

static void M_LoadRooms(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    g_RoomCount = VFile_ReadS16(file);
    LOG_INFO("rooms: %d", g_RoomCount);
    if (g_RoomCount > MAX_ROOMS) {
        Shell_ExitSystem("Too many rooms");
        goto finish;
    }

    g_Rooms = GameBuf_Alloc(sizeof(ROOM) * g_RoomCount, GBUF_ROOMS);
    ASSERT(g_Rooms != NULL);

    for (int32_t i = 0; i < g_RoomCount; i++) {
        ROOM *const r = &g_Rooms[i];

        r->pos.x = VFile_ReadS32(file);
        r->pos.y = 0;
        r->pos.z = VFile_ReadS32(file);

        r->min_floor = VFile_ReadS32(file);
        r->max_ceiling = VFile_ReadS32(file);

        const int32_t data_size = VFile_ReadS32(file);
        r->data = GameBuf_Alloc(sizeof(int16_t) * data_size, GBUF_ROOM_MESH);
        VFile_Read(file, r->data, sizeof(int16_t) * data_size);

        const int16_t num_doors = VFile_ReadS16(file);
        if (num_doors <= 0) {
            r->portals = NULL;
        } else {
            r->portals = GameBuf_Alloc(
                sizeof(PORTAL) * num_doors + sizeof(PORTALS),
                GBUF_ROOM_PORTALS);
            r->portals->count = num_doors;
            VFile_Read(file, r->portals->portal, sizeof(PORTAL) * num_doors);
        }

        r->size.z = VFile_ReadS16(file);
        r->size.x = VFile_ReadS16(file);

        r->sectors = GameBuf_Alloc(
            sizeof(SECTOR) * r->size.z * r->size.x, GBUF_ROOM_SECTORS);
        for (int32_t i = 0; i < r->size.z * r->size.x; i++) {
            SECTOR *const sector = &r->sectors[i];
            sector->idx = VFile_ReadU16(file);
            sector->box = VFile_ReadS16(file);
            sector->portal_room.pit = VFile_ReadU8(file);
            sector->floor.height = VFile_ReadS8(file) * STEP_L;
            sector->portal_room.sky = VFile_ReadU8(file);
            sector->ceiling.height = VFile_ReadS8(file) * STEP_L;
        }

        r->ambient_1 = VFile_ReadS16(file);
        r->ambient_2 = VFile_ReadS16(file);
        r->light_mode = VFile_ReadS16(file);

        r->num_lights = VFile_ReadS16(file);
        if (!r->num_lights) {
            r->lights = NULL;
        } else {
            r->lights =
                GameBuf_Alloc(sizeof(LIGHT) * r->num_lights, GBUF_ROOM_LIGHTS);
            for (int32_t i = 0; i < r->num_lights; i++) {
                LIGHT *const light = &r->lights[i];
                light->pos.x = VFile_ReadS32(file);
                light->pos.y = VFile_ReadS32(file);
                light->pos.z = VFile_ReadS32(file);
                light->intensity_1 = VFile_ReadS16(file);
                light->intensity_2 = VFile_ReadS16(file);
                light->falloff_1 = VFile_ReadS32(file);
                light->falloff_2 = VFile_ReadS32(file);
            }
        }

        r->num_static_meshes = VFile_ReadS16(file);
        if (!r->num_static_meshes) {
            r->static_meshes = NULL;
        } else {
            r->static_meshes = GameBuf_Alloc(
                sizeof(STATIC_MESH) * r->num_static_meshes,
                GBUF_ROOM_STATIC_MESHES);
            for (int32_t i = 0; i < r->num_static_meshes; i++) {
                STATIC_MESH *const mesh = &r->static_meshes[i];
                mesh->pos.x = VFile_ReadS32(file);
                mesh->pos.y = VFile_ReadS32(file);
                mesh->pos.z = VFile_ReadS32(file);
                mesh->rot.y = VFile_ReadS16(file);
                mesh->shade_1 = VFile_ReadS16(file);
                mesh->shade_2 = VFile_ReadS16(file);
                mesh->static_num = VFile_ReadS16(file);
            }
        }

        r->flipped_room = VFile_ReadS16(file);
        r->flags = VFile_ReadU16(file);

        r->bound_active = 0;
        r->bound_left = g_PhdWinMaxX;
        r->bound_top = g_PhdWinMaxY;
        r->bound_bottom = 0;
        r->bound_right = 0;
        r->item_num = NO_ITEM;
        r->effect_num = NO_EFFECT;
    }

    // TODO: store this temporarily in a m_LevelInfo property similar to TR1 and
    // release after parsing.
    const int32_t floor_data_size = VFile_ReadS32(file);
    m_FloorData =
        GameBuf_Alloc(sizeof(int16_t) * floor_data_size, GBUF_FLOOR_DATA);
    VFile_Read(file, m_FloorData, sizeof(int16_t) * floor_data_size);

finish:
    Benchmark_End(benchmark, NULL);
}

static void M_LoadMeshBase(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_meshes = VFile_ReadS32(file);
    LOG_INFO("meshes: %d", num_meshes);
    g_MeshBase = GameBuf_Alloc(sizeof(int16_t) * num_meshes, GBUF_MESHES);
    VFile_Read(file, g_MeshBase, sizeof(int16_t) * num_meshes);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadMeshes(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_mesh_ptrs = VFile_ReadS32(file);
    LOG_INFO("mesh pointers: %d", num_mesh_ptrs);
    int32_t *const mesh_indices =
        (int32_t *)Memory_Alloc(sizeof(int32_t) * num_mesh_ptrs);
    VFile_Read(file, mesh_indices, sizeof(int32_t) * num_mesh_ptrs);

    g_Meshes =
        GameBuf_Alloc(sizeof(int16_t *) * num_mesh_ptrs, GBUF_MESH_POINTERS);
    for (int32_t i = 0; i < num_mesh_ptrs; i++) {
        g_Meshes[i] = &g_MeshBase[mesh_indices[i] / 2];
    }

    Memory_Free(mesh_indices);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnims(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_anims = VFile_ReadS32(file);
    LOG_INFO("anims: %d", num_anims);
    Anim_InitialiseAnims(num_anims);
    Level_ReadAnims(0, num_anims, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimChanges(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_anim_changes = VFile_ReadS32(file);
    LOG_INFO("anim changes: %d", num_anim_changes);
    Anim_InitialiseChanges(num_anim_changes);
    Level_ReadAnimChanges(0, num_anim_changes, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimRanges(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_anim_ranges = VFile_ReadS32(file);
    LOG_INFO("anim ranges: %d", num_anim_ranges);
    Anim_InitialiseRanges(num_anim_ranges);
    Level_ReadAnimRanges(0, num_anim_ranges, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimCommands(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_anim_commands = VFile_ReadS32(file);
    LOG_INFO("anim commands: %d", num_anim_commands);
    Anim_InitialiseCommands(num_anim_commands);
    Level_ReadAnimCommands(0, num_anim_commands, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimBones(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_anim_bones = VFile_ReadS32(file) / ANIM_BONE_SIZE;
    LOG_INFO("anim bones: %d", num_anim_bones);
    Anim_InitialiseBones(num_anim_bones);
    Level_ReadAnimBones(0, num_anim_bones, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimFrames(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_AnimFrameDataLength = VFile_ReadS32(file);
    LOG_INFO("anim frame data size: %d", m_AnimFrameDataLength);
    m_AnimFrameData = Memory_Alloc(sizeof(int16_t) * m_AnimFrameDataLength);
    VFile_Read(file, m_AnimFrameData, sizeof(int16_t) * m_AnimFrameDataLength);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadObjects(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_objects = VFile_ReadS32(file);
    LOG_INFO("objects: %d", num_objects);
    for (int32_t i = 0; i < num_objects; i++) {
        const GAME_OBJECT_ID object_id = VFile_ReadS32(file);
        OBJECT *const object = &g_Objects[object_id];
        object->mesh_count = VFile_ReadS16(file);
        object->mesh_idx = VFile_ReadS16(file);
        object->bone_idx = VFile_ReadS32(file) / ANIM_BONE_SIZE;
        object->frame_ofs = VFile_ReadU32(file);
        object->frame_base = NULL;
        object->anim_idx = VFile_ReadS16(file);
        object->loaded = 1;
    }
    Benchmark_End(benchmark, NULL);
}

static void M_LoadStaticObjects(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_static_objects = VFile_ReadS32(file);
    LOG_INFO("static objects: %d", num_static_objects);
    for (int32_t i = 0; i < num_static_objects; i++) {
        const int32_t static_num = VFile_ReadS32(file);
        STATIC_INFO *static_obj = &g_StaticObjects[static_num];
        static_obj->mesh_idx = VFile_ReadS16(file);
        static_obj->draw_bounds.min_x = VFile_ReadS16(file);
        static_obj->draw_bounds.max_x = VFile_ReadS16(file);
        static_obj->draw_bounds.min_y = VFile_ReadS16(file);
        static_obj->draw_bounds.max_y = VFile_ReadS16(file);
        static_obj->draw_bounds.min_z = VFile_ReadS16(file);
        static_obj->draw_bounds.max_z = VFile_ReadS16(file);
        static_obj->collision_bounds.min_x = VFile_ReadS16(file);
        static_obj->collision_bounds.max_x = VFile_ReadS16(file);
        static_obj->collision_bounds.min_y = VFile_ReadS16(file);
        static_obj->collision_bounds.max_y = VFile_ReadS16(file);
        static_obj->collision_bounds.min_z = VFile_ReadS16(file);
        static_obj->collision_bounds.max_z = VFile_ReadS16(file);
        static_obj->flags = VFile_ReadU16(file);
    }
    Benchmark_End(benchmark, NULL);
}

static void M_LoadTextures(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_textures = VFile_ReadS32(file);
    LOG_INFO("textures: %d", num_textures);
    if (num_textures > MAX_TEXTURES) {
        Shell_ExitSystem("Too many textures");
    }

    g_TextureInfoCount = num_textures;
    for (int32_t i = 0; i < num_textures; i++) {
        PHD_TEXTURE *texture = &g_TextureInfo[i];
        texture->draw_type = VFile_ReadU16(file);
        texture->tex_page = VFile_ReadU16(file);
        for (int32_t j = 0; j < 4; j++) {
            texture->uv[j].u = VFile_ReadU16(file);
            texture->uv[j].v = VFile_ReadU16(file);
        }
    }

    for (int32_t i = 0; i < num_textures; i++) {
        uint16_t *const uv = &g_TextureInfo[i].uv[0].u;
        uint8_t byte = 0;
        for (int32_t j = 0; j < 8; j++) {
            if ((uv[j] & 0x80) != 0) {
                uv[j] |= 0xFF;
                byte |= 1 << j;
            } else {
                uv[j] &= 0xFF00;
            }
        }
        g_LabTextureUVFlag[i] = byte;

        for (int32_t j = 0; j < 4; j++) {
            g_TextureInfo[i].uv_backup[j] = g_TextureInfo[i].uv[j];
        }
    }

    Benchmark_End(benchmark, NULL);
}

static void M_LoadSprites(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_sprites = VFile_ReadS32(file);
    LOG_DEBUG("sprites: %d", num_sprites);
    for (int32_t i = 0; i < num_sprites; i++) {
        PHD_SPRITE *const sprite = &g_PhdSprites[i];
        sprite->tex_page = VFile_ReadU16(file);
        sprite->offset = VFile_ReadU16(file);
        sprite->width = VFile_ReadU16(file);
        sprite->height = VFile_ReadU16(file);
        sprite->x0 = VFile_ReadS16(file);
        sprite->y0 = VFile_ReadS16(file);
        sprite->x1 = VFile_ReadS16(file);
        sprite->y1 = VFile_ReadS16(file);
    }

    const int32_t num_statics = VFile_ReadS32(file);
    LOG_DEBUG("statics: %d", num_statics);
    for (int32_t i = 0; i < num_statics; i++) {
        int32_t object_id = VFile_ReadS32(file);
        if (object_id >= O_NUMBER_OF) {
            object_id -= O_NUMBER_OF;
            STATIC_INFO *const static_object = &g_StaticObjects[object_id];
            VFile_Skip(file, sizeof(int16_t));
            static_object->mesh_idx = VFile_ReadS16(file);
        } else {
            OBJECT *const object = &g_Objects[object_id];
            object->mesh_count = VFile_ReadS16(file);
            object->mesh_idx = VFile_ReadS16(file);
            object->loaded = 1;
        }
    }

    Benchmark_End(benchmark, NULL);
}

static void M_LoadItems(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    const int32_t num_items = VFile_ReadS32(file);
    LOG_DEBUG("items: %d", num_items);
    if (!num_items) {
        g_LevelItemCount = 0;
        goto finish;
    }

    if (num_items > MAX_ITEMS) {
        Shell_ExitSystem("Too many items");
        goto finish;
    }

    g_Items = GameBuf_Alloc(sizeof(ITEM) * MAX_ITEMS, GBUF_ITEMS);
    g_LevelItemCount = num_items;

    Item_InitialiseArray(MAX_ITEMS);

    for (int32_t i = 0; i < num_items; i++) {
        ITEM *const item = &g_Items[i];
        item->object_id = VFile_ReadS16(file);
        item->room_num = VFile_ReadS16(file);
        item->pos.x = VFile_ReadS32(file);
        item->pos.y = VFile_ReadS32(file);
        item->pos.z = VFile_ReadS32(file);
        item->rot.y = VFile_ReadS16(file);
        item->shade_1 = VFile_ReadS16(file);
        item->shade_2 = VFile_ReadS16(file);
        item->flags = VFile_ReadS16(file);
        if (item->object_id < 0 || item->object_id >= O_NUMBER_OF) {
            Shell_ExitSystemFmt(
                "Bad object number (%d) on item %d", item->object_id, i);
            goto finish;
        }
    }

finish:
    Benchmark_End(benchmark, NULL);
}

static void M_LoadDepthQ(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    for (int32_t i = 0; i < 32; i++) {
        VFile_Read(file, g_DepthQTable[i].index, sizeof(uint8_t) * 256);
        g_DepthQTable[i].index[0] = 0;
    }

    for (int32_t i = 0; i < 32; i++) {
        for (int32_t j = 0; j < 256; j++) {
            g_GouraudTable[j].index[i] = g_DepthQTable[i].index[j];
        }
    }

    Benchmark_End(benchmark, NULL);
}

static void M_LoadPalettes(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    VFile_Read(file, g_GamePalette8, sizeof(RGB_888) * 256);
    g_GamePalette8[0].red = 0;
    g_GamePalette8[0].green = 0;
    g_GamePalette8[0].blue = 0;
    for (int32_t i = 1; i < 256; i++) {
        RGB_888 *col = &g_GamePalette8[i];
        col->red = (col->red << 2) | (col->red >> 4);
        col->green = (col->green << 2) | (col->green >> 4);
        col->blue = (col->blue << 2) | (col->blue >> 4);
    }

    struct {
        uint8_t r, g, b, flags;
    } palette_16[256];
    VFile_Read(file, palette_16, 4 * 256);
    for (int32_t i = 0; i < 256; i++) {
        g_GamePalette16[i].r = palette_16[i].r;
        g_GamePalette16[i].g = palette_16[i].g;
        g_GamePalette16[i].b = palette_16[i].b;
    }

    for (int32_t i = 0; i < COLOR_NUMBER_OF; i++) {
        g_NamedColors[i].palette_index = Output_FindColor(
            g_NamedColors[i].rgb.red, g_NamedColors[i].rgb.green,
            g_NamedColors[i].rgb.blue);
    }

    Benchmark_End(benchmark, NULL);
}

static void M_LoadCameras(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_NumCameras = VFile_ReadS32(file);
    LOG_DEBUG("fixed cameras: %d", g_NumCameras);
    if (!g_NumCameras) {
        goto finish;
    }

    g_Camera.fixed =
        GameBuf_Alloc(sizeof(OBJECT_VECTOR) * g_NumCameras, GBUF_CAMERAS);
    for (int32_t i = 0; i < g_NumCameras; i++) {
        OBJECT_VECTOR *const camera = &g_Camera.fixed[i];
        camera->x = VFile_ReadS32(file);
        camera->y = VFile_ReadS32(file);
        camera->z = VFile_ReadS32(file);
        camera->data = VFile_ReadS16(file);
        camera->flags = VFile_ReadS16(file);
    }

finish:
    Benchmark_End(benchmark, NULL);
}

static void M_LoadSoundEffects(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    g_SoundEffectCount = VFile_ReadS32(file);
    LOG_DEBUG("sound effects: %d", g_SoundEffectCount);
    if (!g_SoundEffectCount) {
        goto finish;
    }

    g_SoundEffects = GameBuf_Alloc(
        sizeof(OBJECT_VECTOR) * g_SoundEffectCount, GBUF_SOUND_FX);
    for (int32_t i = 0; i < g_SoundEffectCount; i++) {
        OBJECT_VECTOR *const effect = &g_SoundEffects[i];
        effect->x = VFile_ReadS32(file);
        effect->y = VFile_ReadS32(file);
        effect->z = VFile_ReadS32(file);
        effect->data = VFile_ReadS16(file);
        effect->flags = VFile_ReadS16(file);
    }

finish:
    Benchmark_End(benchmark, NULL);
}

static void M_LoadBoxes(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_BoxCount = VFile_ReadS32(file);
    g_Boxes = GameBuf_Alloc(sizeof(BOX_INFO) * g_BoxCount, GBUF_BOXES);
    for (int32_t i = 0; i < g_BoxCount; i++) {
        BOX_INFO *const box = &g_Boxes[i];
        box->left = VFile_ReadU8(file);
        box->right = VFile_ReadU8(file);
        box->top = VFile_ReadU8(file);
        box->bottom = VFile_ReadU8(file);
        box->height = VFile_ReadS16(file);
        box->overlap_index = VFile_ReadS16(file);
    }

    const int32_t num_overlaps = VFile_ReadS32(file);
    g_Overlap = GameBuf_Alloc(sizeof(uint16_t) * num_overlaps, GBUF_OVERLAPS);
    VFile_Read(file, g_Overlap, sizeof(uint16_t) * num_overlaps);

    for (int32_t i = 0; i < 2; i++) {
        for (int32_t j = 0; j < 4; j++) {
            const bool skip = j == 2
                || (j == 1 && !g_Objects[O_SPIDER].loaded
                    && !g_Objects[O_SKIDOO_ARMED].loaded)
                || (j == 3 && !g_Objects[O_YETI].loaded
                    && !g_Objects[O_WORKER_3].loaded);

            if (skip) {
                VFile_Skip(file, sizeof(int16_t) * g_BoxCount);
                continue;
            }

            g_GroundZone[j][i] =
                GameBuf_Alloc(sizeof(int16_t) * g_BoxCount, GBUF_GROUND_ZONE);
            VFile_Read(file, g_GroundZone[j][i], sizeof(int16_t) * g_BoxCount);
        }

        g_FlyZone[i] =
            GameBuf_Alloc(sizeof(int16_t) * g_BoxCount, GBUF_FLY_ZONE);
        VFile_Read(file, g_FlyZone[i], sizeof(int16_t) * g_BoxCount);
    }

    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimatedTextures(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_ranges = VFile_ReadS32(file);
    g_AnimTextureRanges = GameBuf_Alloc(
        sizeof(int16_t) * num_ranges, GBUF_ANIMATING_TEXTURE_RANGES);
    VFile_Read(file, g_AnimTextureRanges, sizeof(int16_t) * num_ranges);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadCinematic(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_NumCineFrames = VFile_ReadS16(file);
    if (g_NumCineFrames <= 0) {
        goto finish;
    }

    g_CineData = GameBuf_Alloc(
        sizeof(CINE_FRAME) * g_NumCineFrames, GBUF_CINEMATIC_FRAMES);
    for (int32_t i = 0; i < g_NumCineFrames; i++) {
        CINE_FRAME *const frame = &g_CineData[i];
        frame->tx = VFile_ReadS16(file);
        frame->ty = VFile_ReadS16(file);
        frame->tz = VFile_ReadS16(file);
        frame->cx = VFile_ReadS16(file);
        frame->cy = VFile_ReadS16(file);
        frame->cz = VFile_ReadS16(file);
        frame->fov = VFile_ReadS16(file);
        frame->roll = VFile_ReadS16(file);
    }

finish:
    Benchmark_End(benchmark, NULL);
}

static void M_LoadDemo(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_DemoCount = 0;

    // TODO: is the allocation necessary if there's no demo data?
    // TODO: do not hardcode the allocation size
    g_DemoPtr = GameBuf_Alloc(36000, GBUF_DEMO_BUFFER);

    const int32_t demo_size = VFile_ReadU16(file);
    LOG_DEBUG("demo input size: %d", demo_size);
    if (!demo_size) {
        g_IsDemoLoaded = false;
    } else {
        g_IsDemoLoaded = true;
        VFile_Read(file, g_DemoPtr, demo_size);
    }

    Benchmark_End(benchmark, NULL);
}

static void M_LoadSamples(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    int32_t *sample_offsets = NULL;

    Audio_Sample_CloseAll();
    Audio_Sample_UnloadAll();

    VFile_Read(file, g_SampleLUT, sizeof(int16_t) * SFX_NUMBER_OF);
    const int32_t num_sample_infos = VFile_ReadS32(file);
    LOG_DEBUG("sample infos: %d", num_sample_infos);
    if (num_sample_infos == 0) {
        goto finish;
    }

    g_SampleInfos = GameBuf_Alloc(
        sizeof(SAMPLE_INFO) * num_sample_infos, GBUF_SAMPLE_INFOS);
    for (int32_t i = 0; i < num_sample_infos; i++) {
        SAMPLE_INFO *const sample_info = &g_SampleInfos[i];
        sample_info->number = VFile_ReadS16(file);
        sample_info->volume = VFile_ReadS16(file);
        sample_info->randomness = VFile_ReadS16(file);
        sample_info->flags = VFile_ReadS16(file);
    }

    const int32_t num_samples = VFile_ReadS32(file);
    LOG_DEBUG("samples: %d", num_samples);
    if (!num_samples) {
        goto finish;
    }

    sample_offsets = Memory_Alloc(sizeof(int32_t) * num_samples);
    VFile_Read(file, sample_offsets, sizeof(int32_t) * num_samples);

    const char *const file_name = "data\\main.sfx";
    const char *full_path = File_GetFullPath(file_name);
    LOG_DEBUG("Loading samples from %s", full_path);
    MYFILE *const fp = File_Open(full_path, FILE_OPEN_READ);
    Memory_FreePointer(&full_path);

    if (fp == NULL) {
        Shell_ExitSystemFmt("Could not open %s file", file_name);
        goto finish;
    }

    // TODO: refactor these WAVE/RIFF shenanigans
    int32_t sample_id = 0;
    for (int32_t i = 0; sample_id < num_samples; i++) {
        char header[0x2C];
        File_ReadData(fp, header, 0x2C);
        if (*(int32_t *)(header + 0) != 0x46464952
            || *(int32_t *)(header + 8) != 0x45564157
            || *(int32_t *)(header + 36) != 0x61746164) {
            LOG_ERROR("Unexpected sample header for sample %d", i);
            goto finish;
        }
        const int32_t data_size = *(int32_t *)(header + 0x28);
        const int32_t aligned_size = (data_size + 1) & ~1;

        if (sample_offsets[sample_id] != i) {
            File_Seek(fp, aligned_size, FILE_SEEK_CUR);
            continue;
        }

        const size_t sample_data_size = 0x2C + aligned_size;
        char *sample_data = Memory_Alloc(sample_data_size);
        memcpy(sample_data, header, 0x2C);
        File_ReadData(fp, sample_data + 0x2C, aligned_size);

        const bool result =
            Audio_Sample_LoadSingle(sample_id, sample_data, sample_data_size);
        Memory_FreePointer(&sample_data);

        if (!result) {
            goto finish;
        }

        sample_id++;
    }
    File_Close(fp);

finish:
    Memory_FreePointer(&sample_offsets);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadFromFile(const char *const file_name, const int32_t level_num)
{
    LOG_DEBUG("%s (num=%d)", g_GF_LevelNames[level_num], level_num);
    GameBuf_Reset();

    BENCHMARK *const benchmark = Benchmark_Start();

    const char *full_path = File_GetFullPath(file_name);
    strcpy(g_LevelFileName, full_path);
    VFILE *const file = VFile_CreateFromPath(full_path);
    Memory_FreePointer(&full_path);

    const int32_t version = VFile_ReadS32(file);
    if (version > 45) {
        Shell_ExitSystemFmt(
            "FATAL: Level %d (%s) requires a new TOMB2.EXE (version %d) to run",
            level_num, full_path, file_name);
    }

    if (version < 45) {
        Shell_ExitSystemFmt(
            "FATAL: Level %d (%s) is OUT OF DATE (version %d). COPY NEW "
            "EDITORS AND REMAKE LEVEL",
            level_num, full_path, file_name);
    }

    M_LoadPalettes(file);
    M_LoadTexturePages(file);
    VFile_Skip(file, 4);
    M_LoadRooms(file);

    M_LoadMeshBase(file);
    M_LoadMeshes(file);

    M_LoadAnims(file);
    M_LoadAnimChanges(file);
    M_LoadAnimRanges(file);
    M_LoadAnimCommands(file);
    M_LoadAnimBones(file);
    M_LoadAnimFrames(file);

    M_LoadObjects(file);
    Object_SetupAllObjects();

    M_LoadStaticObjects(file);
    M_LoadTextures(file);

    M_LoadSprites(file);
    M_LoadCameras(file);
    M_LoadSoundEffects(file);
    M_LoadBoxes(file);
    M_LoadAnimatedTextures(file);
    M_LoadItems(file);

    M_LoadDepthQ(file);
    M_LoadCinematic(file);
    M_LoadDemo(file);
    M_LoadSamples(file);

    VFile_Close(file);
    Benchmark_End(benchmark, NULL);
}

static void M_CompleteSetup(void)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    // Expand raw floor data into sectors
    Room_ParseFloorData(m_FloorData);
    // TODO: store raw FD temporarily, release here and eliminate g_FloorData

    Inject_AllInjections();

    const int32_t frame_count = Anim_GetTotalFrameCount(m_AnimFrameDataLength);
    Anim_InitialiseFrames(frame_count);
    Anim_LoadFrames(m_AnimFrameData, m_AnimFrameDataLength);
    Memory_FreePointer(&m_AnimFrameData);

    // Must be called after Setup_AllObjects using the cached item
    // count, as individual setups may increment g_LevelItemCount.
    const int32_t item_count = g_LevelItemCount;
    for (int32_t i = 0; i < item_count; i++) {
        Item_Initialise(i);
    }

    Render_Reset(RENDER_RESET_PALETTE | RENDER_RESET_TEXTURES);

    Benchmark_End(benchmark, NULL);
}

bool Level_Load(const char *const file_name, const int32_t level_num)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    for (int32_t i = 0; i < O_NUMBER_OF; i++) {
        OBJECT *const object = Object_GetObject(i);
        object->loaded = false;
    }

    const GAME_FLOW_NEW_LEVEL *const level = &g_GameFlowNew.levels[level_num];
    Inject_Init(level->injections.count, level->injections.data_paths);

    M_LoadFromFile(file_name, level_num);
    M_CompleteSetup();

    Inject_Cleanup();

    Benchmark_End(benchmark, NULL);

    return true;
}
