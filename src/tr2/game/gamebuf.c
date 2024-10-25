#include "game/gamebuf.h"

#include "game/shell.h"
#include "global/vars.h"

#include <libtrx/memory.h>

#include <stddef.h>

static int32_t m_Cap = 0;

static const char *M_GetBufferName(GAME_BUFFER buffer);

static const char *M_GetBufferName(const GAME_BUFFER buffer)
{
    // clang-format off
    switch (buffer) {
        case GBUF_TEMP_ALLOC:               return "Temp Alloc";
        case GBUF_TEXTURE_PAGES:            return "Texture Pages";
        case GBUF_MESH_POINTERS:            return "Mesh Pointers";
        case GBUF_MESHES:                   return "Meshes";
        case GBUF_ANIMS:                    return "Anims";
        case GBUF_STRUCTS:                  return "Structs";
        case GBUF_ANIM_RANGES:              return "Ranges";
        case GBUF_ANIM_COMMANDS:            return "Commands";
        case GBUF_ANIM_BONES:               return "Bones";
        case GBUF_ANIM_FRAMES:              return "Frames";
        case GBUF_ROOM_TEXTURES:            return "Room Textures";
        case GBUF_ROOMS:                    return "Room Infos";
        case GBUF_ROOM_MESH:                return "Room Mesh";
        case GBUF_ROOM_PORTALS:             return "Room Door";
        case GBUF_ROOM_FLOOR:               return "Room Floor";
        case GBUF_ROOM_LIGHTS:              return "Room Lights";
        case GBUF_ROOM_STATIC_MESHES:       return "Room Static Mesh Infos";
        case GBUF_FLOOR_DATA:               return "Floor Data";
        case GBUF_ITEMS:                    return "ITEMS!!";
        case GBUF_CAMERAS:                  return "Cameras";
        case GBUF_SOUND_FX:                 return "Sound FX";
        case GBUF_BOXES:                    return "Boxes";
        case GBUF_OVERLAPS:                 return "Overlaps";
        case GBUF_GROUND_ZONE:              return "GroundZone";
        case GBUF_FLY_ZONE:                 return "FlyZone";
        case GBUF_ANIMATING_TEXTURE_RANGES: return "Animating Texture Ranges";
        case GBUF_CINEMATIC_FRAMES:         return "Cinematic Frames";
        case GBUF_LOAD_DEMO_BUFFER:         return "LoadDemo Buffer";
        case GBUF_SAVE_DEMO_BUFFER:         return "SaveDemo Buffer";
        case GBUF_CINEMATIC_EFFECTS:        return "Cinematic Effects";
        case GBUF_MUMMY_HEAD_TURN:          return "Mummy Head Turn";
        case GBUF_EXTRA_DOOR_STUFF:         return "Extra Door stuff";
        case GBUF_EFFECTS_ARRAY:            return "Effects_Array";
        case GBUF_CREATURE_DATA:            return "Creature Data";
        case GBUF_CREATURE_LOT:             return "Creature LOT";
        case GBUF_SAMPLE_INFOS:             return "Sample Infos";
        case GBUF_SAMPLES:                  return "Samples";
        case GBUF_SAMPLE_OFFSETS:           return "Sample Offsets";
        case GBUF_ROLLING_BALL_STUFF:       return "Rolling Ball Stuff";
        case GBUF_SKIDOO_STUFF:             return "Skidoo Stuff";
        case GBUF_LOAD_PICTURE_BUFFER:      return "Load Piccy Buffer";
        case GBUF_FMV_BUFFERS:              return "FMV Buffers";
        case GBUF_POLYGON_BUFFERS:          return "Polygon Buffers";
        case GBUF_ORDER_TABLES:             return "Order Tables";
        case GBUF_CLUTS:                    return "CLUTs";
        case GBUF_TEXTURE_INFOS:            return "Texture Infos";
        case GBUF_SPRITE_INFOS:             return "Sprite Infos";
        default:                            return "Unknown";
    }
    // clang-format on
}

void GameBuf_Init(const int32_t cap)
{
    m_Cap = cap;
    g_GameBuf_MemBase = Memory_Alloc(cap);
}

void __cdecl GameBuf_Reset(void)
{
    g_GameBuf_MemPtr = g_GameBuf_MemBase;
    g_GameBuf_MemFree = m_Cap;
    g_GameBuf_MemUsed = 0;
}

void __cdecl GameBuf_Shutdown(void)
{
    Memory_FreePointer(&g_GameBuf_MemBase);
    m_Cap = 0;
    g_GameBuf_MemFree = 0;
    g_GameBuf_MemUsed = 0;
}

void *__cdecl GameBuf_Alloc(const size_t alloc_size, const GAME_BUFFER buffer)
{
    const size_t aligned_size = (alloc_size + 3) & ~3;

    if (aligned_size > g_GameBuf_MemFree) {
        Shell_ExitSystemFmt(
            "GameBuf_Alloc(): OUT OF MEMORY %s %d", M_GetBufferName(buffer),
            aligned_size);
    }

    void *result = g_GameBuf_MemPtr;
    g_GameBuf_MemFree -= aligned_size;
    g_GameBuf_MemUsed += aligned_size;
    g_GameBuf_MemPtr += aligned_size;
    return result;
}
