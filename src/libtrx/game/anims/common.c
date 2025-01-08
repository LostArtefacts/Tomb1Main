#include "game/anims/common.h"

#include "game/gamebuf.h"

static ANIM *m_Anims = NULL;
static ANIM_CHANGE *m_Changes = NULL;
ANIM_RANGE *g_AnimRanges = NULL;
static ANIM_BONE *m_Bones = NULL;

void Anim_InitialiseAnims(const int32_t num_anims)
{
    m_Anims = GameBuf_Alloc(sizeof(ANIM) * num_anims, GBUF_ANIMS);
}

void Anim_InitialiseChanges(const int32_t num_changes)
{
    m_Changes =
        GameBuf_Alloc(sizeof(ANIM_CHANGE) * num_changes, GBUF_ANIM_CHANGES);
}

void Anim_InitialiseBones(const int32_t num_bones)
{
    m_Bones = GameBuf_Alloc(sizeof(ANIM_BONE) * num_bones, GBUF_ANIM_BONES);
}

ANIM *Anim_GetAnim(const int32_t anim_idx)
{
    return &m_Anims[anim_idx];
}

ANIM_CHANGE *Anim_GetChange(const int32_t change_idx)
{
    return &m_Changes[change_idx];
}

ANIM_RANGE *Anim_GetRange(const int32_t range_idx)
{
    return &g_AnimRanges[range_idx];
}

ANIM_BONE *Anim_GetBone(const int32_t bone_idx)
{
    return &m_Bones[bone_idx];
}

bool Anim_TestAbsFrameEqual(const int16_t abs_frame, const int16_t frame)
{
    return abs_frame == frame;
}

bool Anim_TestAbsFrameRange(
    const int16_t abs_frame, const int16_t start, const int16_t end)
{
    return abs_frame >= start && abs_frame <= end;
}
