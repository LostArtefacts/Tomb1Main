#pragma once

#include "./types.h"

#include <libtrx/game/game_flow/sequencer.h>

GF_COMMAND
GF_InterpretSequence(
    const GF_LEVEL *level, GF_SEQUENCE_CONTEXT seq_ctx, void *seq_ctx_arg);

GF_COMMAND GF_DoDemoSequence(int32_t demo_num);
GF_COMMAND GF_DoCutsceneSequence(int32_t cutscene_num);
GF_COMMAND GF_PlayAvailableStory(int32_t slot_num);
