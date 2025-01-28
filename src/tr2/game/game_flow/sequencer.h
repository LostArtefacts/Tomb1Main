#pragma once

#include "./types.h"

#include <libtrx/game/game_flow/sequencer.h>

GF_COMMAND GF_InterpretSequence(
    const GF_LEVEL *level, GF_SEQUENCE_CONTEXT seq_ctx, void *user_arg);

bool GF_DoFrontendSequence(void);
GF_COMMAND GF_DoDemoSequence(int32_t demo_num);
GF_COMMAND GF_DoCutsceneSequence(int32_t cutscene_num);
GF_COMMAND GF_DoLevelSequence(
    const GF_LEVEL *start_level, GF_SEQUENCE_CONTEXT seq_ctx);
