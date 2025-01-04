#include "game/anims/common.h"

#include "game/gamebuf.h"

static ANIM_BONE *m_Bones = NULL;

void Anim_InitialiseBones(const int32_t num_bones)
{
    m_Bones = GameBuf_Alloc(sizeof(ANIM_BONE) * num_bones, GBUF_ANIM_BONES);
}

ANIM_BONE *Anim_GetBone(const int32_t bone_idx)
{
    return &m_Bones[bone_idx];
}
