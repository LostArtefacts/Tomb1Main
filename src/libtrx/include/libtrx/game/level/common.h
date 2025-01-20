#pragma once

#include "../../virtual_file.h"

#define ANIM_BONE_SIZE 4

void Level_ReadRoomMesh(int32_t room_num, VFILE *file);
void Level_ReadFloorData(VFILE *file);
void Level_ReadObjectMeshes(
    int32_t num_indices, const int32_t *indices, VFILE *file);
void Level_ReadAnims(int32_t base_idx, int32_t num_anims, VFILE *file);
void Level_ReadAnimChanges(int32_t base_idx, int32_t num_changes, VFILE *file);
void Level_ReadAnimRanges(int32_t base_idx, int32_t num_ranges, VFILE *file);
void Level_InitialiseAnimCommands(int32_t num_cmds);
void Level_ReadAnimCommands(int32_t base_idx, int32_t num_cmds, VFILE *file);
void Level_LoadAnimCommands(void);
void Level_ReadAnimBones(int32_t base_idx, int32_t num_bones, VFILE *file);
void Level_ReadObjects(int32_t num_objects, VFILE *file);
void Level_ReadStaticObjects(int32_t num_objects, VFILE *file);
void Level_ReadObjectTextures(
    int32_t base_idx, int16_t base_page_idx, int32_t num_textures, VFILE *file);
void Level_ReadSpriteTextures(
    int32_t base_idx, int16_t base_page_idx, int32_t num_textures, VFILE *file);
void Level_ReadSpriteSequences(int32_t num_sequences, VFILE *file);
