#include "game/level.h"

#include "game/camera.h"
#include "game/carrier.h"
#include "game/effects.h"
#include "game/game_flow.h"
#include "game/inject.h"
#include "game/inventory_ring/vars.h"
#include "game/items.h"
#include "game/lara/common.h"
#include "game/lot.h"
#include "game/music.h"
#include "game/objects/creatures/mutant.h"
#include "game/objects/creatures/pierre.h"
#include "game/objects/setup.h"
#include "game/output.h"
#include "game/overlay.h"
#include "game/room.h"
#include "game/shell.h"
#include "game/sound.h"
#include "game/stats.h"
#include "game/viewport.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"

#include <libtrx/benchmark.h>
#include <libtrx/config.h>
#include <libtrx/debug.h>
#include <libtrx/game/game_buf.h>
#include <libtrx/game/game_string_table.h>
#include <libtrx/game/level.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>
#include <libtrx/virtual_file.h>

#include <stdio.h>
#include <string.h>

static LEVEL_INFO m_LevelInfo = {};
static INJECTION_INFO *m_InjectionInfo = NULL;

static void M_LoadFromFile(const GAME_FLOW_LEVEL *level, bool is_demo);
static void M_LoadTexturePages(VFILE *file);
static void M_LoadRooms(VFILE *file);
static void M_LoadObjectMeshes(VFILE *file);
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
static void M_LoadCameras(VFILE *file);
static void M_LoadSoundEffects(VFILE *file);
static void M_LoadBoxes(VFILE *file);
static void M_LoadAnimatedTextures(VFILE *file);
static void M_LoadItems(VFILE *file);
static void M_LoadDepthQ(VFILE *file);
static void M_LoadPalette(VFILE *file);
static void M_LoadCinematic(VFILE *file);
static void M_LoadDemo(VFILE *file);
static void M_LoadSamples(VFILE *file);
static void M_CompleteSetup(const GAME_FLOW_LEVEL *level);
static void M_MarkWaterEdgeVertices(void);
static size_t M_CalculateMaxVertices(void);

static void M_LoadFromFile(
    const GAME_FLOW_LEVEL *const level, const bool is_demo)
{
    GameBuf_Reset();

    VFILE *file = VFile_CreateFromPath(level->path);
    if (!file) {
        Shell_ExitSystemFmt("M_LoadFromFile(): Could not open %s", level->path);
    }

    const int32_t version = VFile_ReadS32(file);
    if (version != 32) {
        Shell_ExitSystemFmt(
            "Level %d (%s) is version %d (this game code is version %d)",
            level->num, level->path, version, 32);
    }

    M_LoadTexturePages(file);

    const int32_t file_level_num = VFile_ReadS32(file);
    LOG_INFO("file level num: %d", file_level_num);

    M_LoadRooms(file);
    M_LoadObjectMeshes(file);
    M_LoadAnims(file);
    M_LoadAnimChanges(file);
    M_LoadAnimRanges(file);
    M_LoadAnimCommands(file);
    M_LoadAnimBones(file);
    M_LoadAnimFrames(file);
    M_LoadObjects(file);
    M_LoadStaticObjects(file);
    M_LoadTextures(file);
    M_LoadSprites(file);

    if (is_demo) {
        M_LoadPalette(file);
    }

    M_LoadCameras(file);
    M_LoadSoundEffects(file);
    M_LoadBoxes(file);
    M_LoadAnimatedTextures(file);
    M_LoadItems(file);
    Stats_ObserveItemsLoad();
    M_LoadDepthQ(file);

    if (!is_demo) {
        M_LoadPalette(file);
    }

    M_LoadCinematic(file);
    M_LoadDemo(file);
    M_LoadSamples(file);

    VFile_Close(file);
}

static void M_LoadTexturePages(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.texture_page_count = VFile_ReadS32(file);
    LOG_INFO("%d texture pages", m_LevelInfo.texture_page_count);
    m_LevelInfo.texture_palette_page_ptrs =
        Memory_Alloc(m_LevelInfo.texture_page_count * PAGE_SIZE);
    VFile_Read(
        file, m_LevelInfo.texture_palette_page_ptrs,
        PAGE_SIZE * m_LevelInfo.texture_page_count);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadRooms(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_RoomCount = VFile_ReadU16(file);
    LOG_INFO("%d rooms", g_RoomCount);

    g_RoomInfo = GameBuf_Alloc(sizeof(ROOM) * g_RoomCount, GBUF_ROOMS);
    int i = 0;
    for (ROOM *r = g_RoomInfo; i < g_RoomCount; i++, r++) {
        // Room position
        r->pos.x = VFile_ReadS32(file);
        r->pos.y = 0;
        r->pos.z = VFile_ReadS32(file);

        // Room floor/ceiling
        r->min_floor = VFile_ReadS32(file);
        r->max_ceiling = VFile_ReadS32(file);

        // Room mesh
        Level_ReadRoomMesh(i, file);

        // Doors
        const uint16_t num_doors = VFile_ReadS16(file);
        if (!num_doors) {
            r->portals = NULL;
        } else {
            r->portals = GameBuf_Alloc(
                sizeof(uint16_t) + sizeof(PORTAL) * num_doors,
                GBUF_ROOM_PORTALS);
            r->portals->count = num_doors;
            for (int32_t j = 0; j < num_doors; j++) {
                PORTAL *const portal = &r->portals->portal[j];
                portal->room_num = VFile_ReadS16(file);
                portal->normal.x = VFile_ReadS16(file);
                portal->normal.y = VFile_ReadS16(file);
                portal->normal.z = VFile_ReadS16(file);
                for (int32_t k = 0; k < 4; k++) {
                    portal->vertex[k].x = VFile_ReadS16(file);
                    portal->vertex[k].y = VFile_ReadS16(file);
                    portal->vertex[k].z = VFile_ReadS16(file);
                }
            }
        }

        // Room floor
        r->size.z = VFile_ReadS16(file);
        r->size.x = VFile_ReadS16(file);
        const int32_t sector_count = r->size.x * r->size.z;
        r->sectors =
            GameBuf_Alloc(sizeof(SECTOR) * sector_count, GBUF_ROOM_SECTORS);
        for (int32_t j = 0; j < sector_count; j++) {
            SECTOR *const sector = &r->sectors[j];
            sector->idx = VFile_ReadU16(file);
            sector->box = VFile_ReadS16(file);
            sector->portal_room.pit = VFile_ReadU8(file);
            const int8_t floor_clicks = VFile_ReadS8(file);
            sector->portal_room.sky = VFile_ReadU8(file);
            const int8_t ceiling_clicks = VFile_ReadS8(file);

            sector->floor.height = floor_clicks * STEP_L;
            sector->ceiling.height = ceiling_clicks * STEP_L;
        }

        // Room lights
        r->ambient = VFile_ReadS16(file);
        r->light_mode = RLM_NORMAL;
        r->num_lights = VFile_ReadS16(file);
        if (!r->num_lights) {
            r->lights = NULL;
        } else {
            r->lights =
                GameBuf_Alloc(sizeof(LIGHT) * r->num_lights, GBUF_ROOM_LIGHTS);
            for (int32_t j = 0; j < r->num_lights; j++) {
                LIGHT *light = &r->lights[j];
                light->pos.x = VFile_ReadS32(file);
                light->pos.y = VFile_ReadS32(file);
                light->pos.z = VFile_ReadS32(file);
                light->shade.value_1 = VFile_ReadS16(file);
                light->falloff.value_1 = VFile_ReadS32(file);
            }
        }

        // Static mesh infos
        r->num_static_meshes = VFile_ReadS16(file);
        if (r->num_static_meshes == 0) {
            r->static_meshes = NULL;
        } else {
            r->static_meshes = GameBuf_Alloc(
                sizeof(STATIC_MESH) * r->num_static_meshes,
                GBUF_ROOM_STATIC_MESHES);
            for (int32_t j = 0; j < r->num_static_meshes; j++) {
                STATIC_MESH *const mesh = &r->static_meshes[j];
                mesh->pos.x = VFile_ReadS32(file);
                mesh->pos.y = VFile_ReadS32(file);
                mesh->pos.z = VFile_ReadS32(file);
                mesh->rot.y = VFile_ReadS16(file);
                mesh->shade.value_1 = VFile_ReadU16(file);
                mesh->static_num = VFile_ReadS16(file);
            }
        }

        // Flipped (alternative) room
        r->flipped_room = VFile_ReadS16(file);

        // Room flags
        r->flags = VFile_ReadU16(file);

        // Initialise some variables
        r->bound_active = 0;
        r->bound_left = Viewport_GetMaxX();
        r->bound_top = Viewport_GetMaxY();
        r->bound_bottom = 0;
        r->bound_right = 0;
        r->item_num = NO_ITEM;
        r->effect_num = NO_EFFECT;
    }

    Room_InitialiseFlipStatus();
    Level_ReadFloorData(file);

    Benchmark_End(benchmark, NULL);
}

static void M_LoadObjectMeshes(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.mesh_count = VFile_ReadS32(file);
    LOG_INFO("%d object mesh data", m_LevelInfo.mesh_count);

    const size_t data_start_pos = VFile_GetPos(file);
    VFile_Skip(file, m_LevelInfo.mesh_count * sizeof(int16_t));

    m_LevelInfo.mesh_ptr_count = VFile_ReadS32(file);
    LOG_INFO("%d object mesh indices", m_LevelInfo.mesh_ptr_count);
    const int32_t alloc_size = m_LevelInfo.mesh_ptr_count * sizeof(int32_t);
    int32_t *mesh_indices = Memory_Alloc(alloc_size);
    VFile_Read(file, mesh_indices, alloc_size);

    const size_t end_pos = VFile_GetPos(file);
    VFile_SetPos(file, data_start_pos);

    Object_InitialiseMeshes(
        m_LevelInfo.mesh_ptr_count + m_InjectionInfo->mesh_ptr_count);
    Level_ReadObjectMeshes(m_LevelInfo.mesh_ptr_count, mesh_indices, file);

    VFile_SetPos(file, end_pos);
    Memory_FreePointer(&mesh_indices);

    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnims(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.anim_count = VFile_ReadS32(file);
    LOG_INFO("%d anims", m_LevelInfo.anim_count);
    Anim_InitialiseAnims(m_LevelInfo.anim_count + m_InjectionInfo->anim_count);
    Level_ReadAnims(0, m_LevelInfo.anim_count, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimChanges(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.anim_change_count = VFile_ReadS32(file);
    LOG_INFO("%d anim changes", m_LevelInfo.anim_change_count);
    Anim_InitialiseChanges(
        m_LevelInfo.anim_change_count + m_InjectionInfo->anim_change_count);
    Level_ReadAnimChanges(0, m_LevelInfo.anim_change_count, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimRanges(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.anim_range_count = VFile_ReadS32(file);
    LOG_INFO("%d anim ranges", m_LevelInfo.anim_range_count);
    Anim_InitialiseRanges(
        m_LevelInfo.anim_range_count + m_InjectionInfo->anim_range_count);
    Level_ReadAnimRanges(0, m_LevelInfo.anim_range_count, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimCommands(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.anim_command_count = VFile_ReadS32(file);
    LOG_INFO("%d anim commands", m_LevelInfo.anim_command_count);
    Level_InitialiseAnimCommands(
        m_LevelInfo.anim_command_count + m_InjectionInfo->anim_cmd_count);
    Level_ReadAnimCommands(0, m_LevelInfo.anim_command_count, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimBones(VFILE *const file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.anim_bone_count = VFile_ReadS32(file) / ANIM_BONE_SIZE;
    LOG_INFO("%d anim bones", m_LevelInfo.anim_bone_count);
    Anim_InitialiseBones(
        m_LevelInfo.anim_bone_count + m_InjectionInfo->anim_bone_count);
    Level_ReadAnimBones(0, m_LevelInfo.anim_bone_count, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimFrames(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t raw_data_count = VFile_ReadS32(file);
    m_LevelInfo.anim_frame_data_count = raw_data_count;
    LOG_INFO("%d raw anim frames", m_LevelInfo.anim_frame_data_count);
    m_LevelInfo.anim_frame_data = Memory_Alloc(
        sizeof(int16_t)
        * (raw_data_count + m_InjectionInfo->anim_frame_data_count));
    VFile_Read(
        file, m_LevelInfo.anim_frame_data, sizeof(int16_t) * raw_data_count);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadObjects(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_objects = VFile_ReadS32(file);
    LOG_INFO("%d objects", num_objects);
    Level_ReadObjects(num_objects, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadStaticObjects(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const int32_t num_static_objects = VFile_ReadS32(file);
    LOG_INFO("%d static objects", num_static_objects);
    Level_ReadStaticObjects(num_static_objects, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadTextures(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.texture_count = VFile_ReadS32(file);
    LOG_INFO("%d object textures", m_LevelInfo.texture_count);
    Level_ReadObjectTextures(0, 0, m_LevelInfo.texture_count, file);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadSprites(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.sprite_info_count = VFile_ReadS32(file);
    LOG_DEBUG("sprite textures: %d", m_LevelInfo.sprite_info_count);
    Level_ReadSpriteTextures(0, 0, m_LevelInfo.sprite_info_count, file);

    const int32_t num_sequences = VFile_ReadS32(file);
    LOG_DEBUG("sprite sequences: %d", num_sequences);
    Level_ReadSpriteSequences(num_sequences, file);

    Benchmark_End(benchmark, NULL);
}

static void M_LoadCameras(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_NumberCameras = VFile_ReadS32(file);
    LOG_INFO("%d cameras", g_NumberCameras);
    if (g_NumberCameras != 0) {
        g_Camera.fixed = GameBuf_Alloc(
            sizeof(OBJECT_VECTOR) * g_NumberCameras, GBUF_CAMERAS);
        if (!g_Camera.fixed) {
            Shell_ExitSystem("Error allocating the fixed cameras.");
        }
        for (int32_t i = 0; i < g_NumberCameras; i++) {
            OBJECT_VECTOR *camera = &g_Camera.fixed[i];
            camera->x = VFile_ReadS32(file);
            camera->y = VFile_ReadS32(file);
            camera->z = VFile_ReadS32(file);
            camera->data = VFile_ReadS16(file);
            camera->flags = VFile_ReadS16(file);
        }
    }
    Benchmark_End(benchmark, NULL);
}

static void M_LoadSoundEffects(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_NumberSoundEffects = VFile_ReadS32(file);
    LOG_INFO("%d sound effects", g_NumberSoundEffects);
    if (g_NumberSoundEffects != 0) {
        g_SoundEffectsTable = GameBuf_Alloc(
            sizeof(OBJECT_VECTOR) * g_NumberSoundEffects, GBUF_SOUND_FX);
        if (!g_SoundEffectsTable) {
            Shell_ExitSystem("Error allocating the sound effects table.");
        }
        for (int32_t i = 0; i < g_NumberSoundEffects; i++) {
            OBJECT_VECTOR *sound = &g_SoundEffectsTable[i];
            sound->x = VFile_ReadS32(file);
            sound->y = VFile_ReadS32(file);
            sound->z = VFile_ReadS32(file);
            sound->data = VFile_ReadS16(file);
            sound->flags = VFile_ReadS16(file);
        }
    }
    Benchmark_End(benchmark, NULL);
}

static void M_LoadBoxes(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_NumberBoxes = VFile_ReadS32(file);
    g_Boxes = GameBuf_Alloc(sizeof(BOX_INFO) * g_NumberBoxes, GBUF_BOXES);
    for (int32_t i = 0; i < g_NumberBoxes; i++) {
        BOX_INFO *box = &g_Boxes[i];
        box->left = VFile_ReadS32(file);
        box->right = VFile_ReadS32(file);
        box->top = VFile_ReadS32(file);
        box->bottom = VFile_ReadS32(file);
        box->height = VFile_ReadS16(file);
        box->overlap_index = VFile_ReadS16(file);
    }

    m_LevelInfo.overlap_count = VFile_ReadS32(file);
    g_Overlap = GameBuf_Alloc(
        sizeof(uint16_t) * m_LevelInfo.overlap_count, GBUF_OVERLAPS);
    VFile_Read(file, g_Overlap, sizeof(uint16_t) * m_LevelInfo.overlap_count);

    for (int i = 0; i < 2; i++) {
        g_GroundZone[i] =
            GameBuf_Alloc(sizeof(int16_t) * g_NumberBoxes, GBUF_GROUND_ZONE);
        VFile_Read(file, g_GroundZone[i], sizeof(int16_t) * g_NumberBoxes);

        g_GroundZone2[i] =
            GameBuf_Alloc(sizeof(int16_t) * g_NumberBoxes, GBUF_GROUND_ZONE);
        VFile_Read(file, g_GroundZone2[i], sizeof(int16_t) * g_NumberBoxes);

        g_FlyZone[i] =
            GameBuf_Alloc(sizeof(int16_t) * g_NumberBoxes, GBUF_FLY_ZONE);
        VFile_Read(file, g_FlyZone[i], sizeof(int16_t) * g_NumberBoxes);
    }

    Benchmark_End(benchmark, NULL);
}

static void M_LoadAnimatedTextures(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.anim_texture_range_count = VFile_ReadS32(file);
    size_t end_position = VFile_GetPos(file)
        + m_LevelInfo.anim_texture_range_count * sizeof(int16_t);

    const int16_t num_ranges = VFile_ReadS16(file);
    LOG_INFO("%d animated texture ranges", num_ranges);
    if (!num_ranges) {
        g_AnimTextureRanges = NULL;
        goto cleanup;
    }

    g_AnimTextureRanges = GameBuf_Alloc(
        sizeof(TEXTURE_RANGE) * num_ranges, GBUF_ANIMATING_TEXTURE_RANGES);
    for (int32_t i = 0; i < num_ranges; i++) {
        TEXTURE_RANGE *range = &g_AnimTextureRanges[i];
        range->next_range =
            i == num_ranges - 1 ? NULL : &g_AnimTextureRanges[i + 1];

        // Level data is tied to the original logic in Output_AnimateTextures
        // and hence stores one less than the actual count here.
        range->num_textures = VFile_ReadS16(file);
        range->num_textures++;

        range->textures = GameBuf_Alloc(
            sizeof(int16_t) * range->num_textures,
            GBUF_ANIMATING_TEXTURE_RANGES);
        VFile_Read(
            file, range->textures, sizeof(int16_t) * range->num_textures);
    }

cleanup: {
    // Ensure to read everything intended by the level compiler, even if it
    // does not wholly contain accurate texture data.
    const int32_t skip_length = end_position - VFile_GetPos(file);
    if (skip_length > 0) {
        VFile_Skip(file, skip_length);
    }
    Benchmark_End(benchmark, NULL);
}
}

static void M_LoadItems(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    m_LevelInfo.item_count = VFile_ReadS32(file);

    LOG_INFO("%d items", m_LevelInfo.item_count);

    if (m_LevelInfo.item_count) {
        if (m_LevelInfo.item_count > MAX_ITEMS) {
            Shell_ExitSystem("M_LoadItems(): Too Many g_Items being Loaded!!");
        }

        g_Items = GameBuf_Alloc(sizeof(ITEM) * MAX_ITEMS, GBUF_ITEMS);
        g_LevelItemCount = m_LevelInfo.item_count;
        Item_InitialiseArray(MAX_ITEMS);

        for (int i = 0; i < m_LevelInfo.item_count; i++) {
            ITEM *item = &g_Items[i];
            item->object_id = VFile_ReadS16(file);
            item->room_num = VFile_ReadS16(file);
            item->pos.x = VFile_ReadS32(file);
            item->pos.y = VFile_ReadS32(file);
            item->pos.z = VFile_ReadS32(file);
            item->rot.y = VFile_ReadS16(file);
            item->shade.value_1 = VFile_ReadS16(file);
            item->flags = VFile_ReadU16(file);

            if (item->object_id < 0 || item->object_id >= O_NUMBER_OF) {
                Shell_ExitSystemFmt(
                    "M_LoadItems(): Bad Object number (%d) on Item %d",
                    item->object_id, i);
            }
        }
    }

    Benchmark_End(benchmark, NULL);
}

static void M_LoadDepthQ(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    LOG_INFO("");
    VFile_Skip(file, sizeof(uint8_t) * 32 * 256);
    Benchmark_End(benchmark, NULL);
}

static void M_LoadPalette(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    LOG_INFO("");
    m_LevelInfo.palette_size = 256;
    m_LevelInfo.palette =
        Memory_Alloc(sizeof(RGB_888) * m_LevelInfo.palette_size);
    for (int32_t i = 0; i < 256; i++) {
        m_LevelInfo.palette[i].r = VFile_ReadU8(file);
        m_LevelInfo.palette[i].g = VFile_ReadU8(file);
        m_LevelInfo.palette[i].b = VFile_ReadU8(file);
    }
    m_LevelInfo.palette[0].r = 0;
    m_LevelInfo.palette[0].g = 0;
    m_LevelInfo.palette[0].b = 0;
    for (int i = 1; i < 256; i++) {
        m_LevelInfo.palette[i].r *= 4;
        m_LevelInfo.palette[i].g *= 4;
        m_LevelInfo.palette[i].b *= 4;
    }
    Benchmark_End(benchmark, NULL);
}

static void M_LoadCinematic(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    g_NumCineFrames = VFile_ReadS16(file);
    LOG_INFO("%d cinematic frames", g_NumCineFrames);
    if (g_NumCineFrames != 0) {
        g_CineCamera = GameBuf_Alloc(
            sizeof(CINE_CAMERA) * g_NumCineFrames, GBUF_CINEMATIC_FRAMES);
        for (int32_t i = 0; i < g_NumCineFrames; i++) {
            CINE_CAMERA *camera = &g_CineCamera[i];
            camera->tx = VFile_ReadS16(file);
            camera->ty = VFile_ReadS16(file);
            camera->tz = VFile_ReadS16(file);
            camera->cx = VFile_ReadS16(file);
            camera->cy = VFile_ReadS16(file);
            camera->cz = VFile_ReadS16(file);
            camera->fov = VFile_ReadS16(file);
            camera->roll = VFile_ReadS16(file);
        }
    }
    Benchmark_End(benchmark, NULL);
}

static void M_LoadDemo(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    const uint16_t size = VFile_ReadU16(file);
    LOG_INFO("demo buffer size: %d", size);
    if (size != 0) {
        g_DemoData =
            GameBuf_Alloc((size + 1) * sizeof(uint32_t), GBUF_DEMO_BUFFER);
        VFile_Read(file, g_DemoData, size);
        g_DemoData[size] = -1;
    } else {
        g_DemoData = NULL;
    }
    Benchmark_End(benchmark, NULL);
}

static void M_LoadSamples(VFILE *file)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    VFile_Read(file, g_SampleLUT, sizeof(int16_t) * MAX_SAMPLES);
    m_LevelInfo.sample_info_count = VFile_ReadS32(file);
    LOG_INFO("%d sample infos", m_LevelInfo.sample_info_count);
    if (!m_LevelInfo.sample_info_count) {
        Shell_ExitSystem("No Sample Infos");
    }

    g_SampleInfos = GameBuf_Alloc(
        sizeof(SAMPLE_INFO)
            * (m_LevelInfo.sample_info_count + m_InjectionInfo->sfx_count),
        GBUF_SAMPLE_INFOS);
    for (int32_t i = 0; i < m_LevelInfo.sample_info_count; i++) {
        SAMPLE_INFO *sample_info = &g_SampleInfos[i];
        sample_info->number = VFile_ReadS16(file);
        sample_info->volume = VFile_ReadS16(file);
        sample_info->randomness = VFile_ReadS16(file);
        sample_info->flags = VFile_ReadS16(file);
    }

    m_LevelInfo.sample_data_size = VFile_ReadS32(file);
    LOG_INFO("%d sample data size", m_LevelInfo.sample_data_size);
    if (!m_LevelInfo.sample_data_size) {
        Shell_ExitSystem("No Sample Data");
    }

    m_LevelInfo.sample_data = GameBuf_Alloc(
        m_LevelInfo.sample_data_size + m_InjectionInfo->sfx_data_size,
        GBUF_SAMPLES);
    VFile_Read(
        file, m_LevelInfo.sample_data,
        sizeof(char) * m_LevelInfo.sample_data_size);

    m_LevelInfo.sample_count = VFile_ReadS32(file);
    LOG_INFO("%d samples", m_LevelInfo.sample_count);
    if (!m_LevelInfo.sample_count) {
        Shell_ExitSystem("No Samples");
    }

    m_LevelInfo.sample_offsets = Memory_Alloc(
        sizeof(int32_t)
        * (m_LevelInfo.sample_count + m_InjectionInfo->sample_count));
    VFile_Read(
        file, m_LevelInfo.sample_offsets,
        sizeof(int32_t) * m_LevelInfo.sample_count);

    Benchmark_End(benchmark, NULL);
}

static void M_CompleteSetup(const GAME_FLOW_LEVEL *const level)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    // Expand paletted texture data to RGB
    m_LevelInfo.texture_rgb_page_ptrs = Memory_Alloc(
        m_LevelInfo.texture_page_count * PAGE_SIZE * sizeof(RGBA_8888));
    RGBA_8888 *output = m_LevelInfo.texture_rgb_page_ptrs;
    const uint8_t *input = m_LevelInfo.texture_palette_page_ptrs;
    for (int32_t i = 0; i < m_LevelInfo.texture_page_count; i++) {
        for (int32_t j = 0; j < PAGE_SIZE; j++) {
            const uint8_t index = *input++;
            if (index == 0) {
                output->r = 0;
                output->g = 0;
                output->b = 0;
                output->a = 0;
            } else {
                RGB_888 pix = m_LevelInfo.palette[index];
                output->r = pix.r;
                output->g = pix.g;
                output->b = pix.b;
                output->a = 255;
            }
            output++;
        }
    }

    // We inject explosions sprites and sounds, although in the original game,
    // some levels lack them, resulting in no audio or visual effects when
    // killing mutants. This is to maintain that feature.
    Mutant_ToggleExplosions(g_Objects[O_EXPLOSION_1].loaded);

    Inject_AllInjections(&m_LevelInfo);

    const int32_t frame_count =
        Anim_GetTotalFrameCount(m_LevelInfo.anim_frame_data_count);
    Anim_InitialiseFrames(frame_count);
    Anim_LoadFrames(
        m_LevelInfo.anim_frame_data, m_LevelInfo.anim_frame_data_count);
    Memory_FreePointer(&m_LevelInfo.anim_frame_data);

    Level_LoadAnimCommands();

    M_MarkWaterEdgeVertices();

    // Must be called post-injection to allow for floor data changes.
    Stats_ObserveRoomsLoad();

    // Must be called after all animations, meshes etc are initialised.
    Object_SetupAllObjects();

    // Must be called after Setup_AllObjects using the cached item
    // count, as individual setups may increment g_LevelItemCount.
    for (int i = 0; i < m_LevelInfo.item_count; i++) {
        Item_Initialise(i);
    }

    // Configure enemies who carry and drop items
    Carrier_InitialiseLevel(level);

    const size_t max_vertices = M_CalculateMaxVertices();
    LOG_INFO("Maximum vertices: %d", max_vertices);
    Output_ReserveVertexBuffer(max_vertices);

    // Move the prepared texture pages into g_TexturePagePtrs.
    RGBA_8888 *final_texture_data = GameBuf_Alloc(
        m_LevelInfo.texture_page_count * PAGE_SIZE * sizeof(RGBA_8888),
        GBUF_TEXTURE_PAGES);
    memcpy(
        final_texture_data, m_LevelInfo.texture_rgb_page_ptrs,
        m_LevelInfo.texture_page_count * PAGE_SIZE * sizeof(RGBA_8888));
    for (int i = 0; i < m_LevelInfo.texture_page_count; i++) {
        g_TexturePagePtrs[i] = &final_texture_data[i * PAGE_SIZE];
    }
    Output_DownloadTextures(m_LevelInfo.texture_page_count);
    Output_SetPalette(m_LevelInfo.palette, m_LevelInfo.palette_size);

    // Initialise the sound effects.
    size_t *sample_sizes =
        Memory_Alloc(sizeof(size_t) * m_LevelInfo.sample_count);
    const char **sample_pointers =
        Memory_Alloc(sizeof(char *) * m_LevelInfo.sample_count);
    for (int i = 0; i < m_LevelInfo.sample_count; i++) {
        sample_pointers[i] =
            m_LevelInfo.sample_data + m_LevelInfo.sample_offsets[i];
    }

    // NOTE: this assumes that sample pointers are sorted
    for (int i = 0; i < m_LevelInfo.sample_count; i++) {
        int current_offset = m_LevelInfo.sample_offsets[i];
        int next_offset = i + 1 >= m_LevelInfo.sample_count
            ? m_LevelInfo.sample_data_size
            : m_LevelInfo.sample_offsets[i + 1];
        sample_sizes[i] = next_offset - current_offset;
    }

    Sound_LoadSamples(m_LevelInfo.sample_count, sample_pointers, sample_sizes);

    Memory_FreePointer(&sample_pointers);
    Memory_FreePointer(&sample_sizes);

    Benchmark_End(benchmark, NULL);
}

static void M_MarkWaterEdgeVertices(void)
{
    if (!g_Config.visuals.fix_texture_issues) {
        return;
    }

    BENCHMARK *const benchmark = Benchmark_Start();
    for (int32_t i = 0; i < Room_GetTotalCount(); i++) {
        const ROOM *const room = Room_Get(i);
        const int32_t y_test =
            (room->flags & RF_UNDERWATER) ? room->max_ceiling : room->min_floor;
        for (int32_t j = 0; j < room->mesh.num_vertices; j++) {
            ROOM_VERTEX *const vertex = &room->mesh.vertices[j];
            if (vertex->pos.y == y_test) {
                vertex->flags |= NO_VERT_MOVE;
            }
        }
    }

    Benchmark_End(benchmark, NULL);
}

static size_t M_CalculateMaxVertices(void)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    int32_t max_vertices = 0;
    for (int32_t i = 0; i < O_NUMBER_OF; i++) {
        const OBJECT *const object = Object_GetObject(i);
        if (!object->loaded) {
            continue;
        }

        for (int32_t j = 0; j < object->mesh_count; j++) {
            const OBJECT_MESH *const mesh =
                Object_GetMesh(object->mesh_idx + j);
            max_vertices = MAX(max_vertices, mesh->num_vertices);
        }
    }

    for (int32_t i = 0; i < MAX_STATIC_OBJECTS; i++) {
        const STATIC_OBJECT_3D *static_info = Object_GetStaticObject3D(i);
        if (!static_info->loaded) {
            continue;
        }

        const OBJECT_MESH *const mesh = Object_GetMesh(static_info->mesh_idx);
        max_vertices = MAX(max_vertices, mesh->num_vertices);
    }

    for (int32_t i = 0; i < Room_GetTotalCount(); i++) {
        const ROOM *const room = Room_Get(i);
        max_vertices = MAX(max_vertices, room->mesh.num_vertices);
    }

    Benchmark_End(benchmark, NULL);
    return max_vertices;
}

void Level_Load(const GAME_FLOW_LEVEL *const level)
{
    LOG_INFO("%d (%s)", level->num, level->path);
    BENCHMARK *const benchmark = Benchmark_Start();

    // clean previous level data
    Memory_FreePointer(&m_LevelInfo.texture_palette_page_ptrs);
    Memory_FreePointer(&m_LevelInfo.texture_rgb_page_ptrs);
    Memory_FreePointer(&m_LevelInfo.sample_offsets);
    Memory_FreePointer(&m_LevelInfo.palette);
    Memory_FreePointer(&m_InjectionInfo);

    m_InjectionInfo = Memory_Alloc(sizeof(INJECTION_INFO));
    Inject_Init(
        level->injections.count, level->injections.data_paths, m_InjectionInfo);

    const bool is_demo =
        (level->type == GFL_TITLE_DEMO_PC) | (level->type == GFL_LEVEL_DEMO_PC);

    M_LoadFromFile(level, is_demo);
    M_CompleteSetup(level);

    Inject_Cleanup();

    Output_SetWaterColor(&level->settings.water_color);
    Output_SetDrawDistFade(level->settings.draw_distance_fade * WALL_L);
    Output_SetDrawDistMax(level->settings.draw_distance_max * WALL_L);
    Output_SetSkyboxEnabled(
        g_Config.visuals.enable_skybox && g_Objects[O_SKYBOX].loaded);

    Benchmark_End(benchmark, NULL);
}

bool Level_Initialise(const GAME_FLOW_LEVEL *const level)
{
    BENCHMARK *const benchmark = Benchmark_Start();
    LOG_DEBUG("num=%d (%s)", level->num, level->path);

    // loading a save can override it to false
    g_GameInfo.death_counter_supported = true;

    g_GameInfo.select_level_num = -1;
    const int32_t level_num = level->num;
    RESUME_INFO *const resume = GF_GetResumeInfo(level);
    if (resume != NULL) {
        resume->stats.timer = 0;
        resume->stats.secret_flags = 0;
        resume->stats.secret_count = 0;
        resume->stats.pickup_count = 0;
        resume->stats.kill_count = 0;
        resume->stats.death_count = 0;
    }

    g_LevelComplete = false;
    g_CurrentLevel = level_num;
    g_FlipEffect = -1;

    Overlay_HideGameInfo();

    g_FlipStatus = 0;
    for (int32_t i = 0; i < MAX_FLIP_MAPS; i++) {
        g_FlipMapTable[i] = 0;
    }

    for (int32_t i = 0; i < MAX_CD_TRACKS; i++) {
        g_MusicTrackFlags[i] = 0;
    }

    /* Clear Object Loaded flags */
    for (int32_t i = 0; i < O_NUMBER_OF; i++) {
        Object_GetObject(i)->loaded = false;
    }
    for (int32_t i = 0; i < MAX_STATIC_OBJECTS; i++) {
        Object_GetStaticObject2D(i)->loaded = false;
        Object_GetStaticObject3D(i)->loaded = false;
    }

    Camera_Reset();
    Pierre_Reset();

    Lara_InitialiseLoad(NO_ITEM);
    Level_Load(level);
    GameStringTable_Apply(level);

    if (g_Lara.item_num != NO_ITEM) {
        Lara_Initialise(level);
    }

    Effect_InitialiseArray();
    LOT_InitialiseArray();

    Overlay_Init();
    Overlay_BarSetHealthTimer(100);

    Music_Stop();
    Music_SetVolume(g_Config.audio.music_volume);
    Sound_ResetEffects();

    const bool disable_music = level_num == g_GameFlow.title_level_num
        && !g_Config.audio.enable_music_in_menu;
    if (level->music_track >= 0 && !disable_music) {
        Music_PlayLooped(level->music_track);
    }

    Viewport_SetFOV(-1);

    g_Camera.underwater = false;
    Benchmark_End(benchmark, NULL);
    return true;
}

const LEVEL_INFO *Level_GetInfo(void)
{
    return &m_LevelInfo;
}
