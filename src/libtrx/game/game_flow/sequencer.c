#include "game/game_flow/sequencer.h"

#include "debug.h"
#include "enum_map.h"
#include "game/game_flow/sequencer_priv.h"

GF_COMMAND GF_InterpretSequence(
    const GF_LEVEL *const level, GF_SEQUENCE_CONTEXT seq_ctx,
    void *const seq_ctx_arg)
{
    ASSERT(level != nullptr);
    LOG_DEBUG(
        "running sequence for level=%d type=%d seq_ctx=%d", level->num,
        level->type, seq_ctx);

    GF_PreSequenceHook();

    GF_COMMAND gf_cmd = { .action = GF_EXIT_TO_TITLE };

    const GF_SEQUENCE *const sequence = &level->sequence;
    for (int32_t i = 0; i < sequence->length; i++) {
        const GF_SEQUENCE_EVENT *const event = &sequence->events[i];
        LOG_DEBUG(
            "event type=%s(%d) data=0x%x",
            ENUM_MAP_TO_STRING(GF_SEQUENCE_EVENT_TYPE, event->type),
            event->type, event->data);

        if (GF_ShouldSkipSequenceEvent(level, event)) {
            continue;
        }

        // Handle the event
        if (event->type < GFS_NUMBER_OF
            && GF_GetSequenceEventHandler(event->type) != nullptr) {
            gf_cmd = GF_GetSequenceEventHandler(event->type)(
                level, event, seq_ctx, seq_ctx_arg);
            LOG_DEBUG(
                "event type=%s(%d) data=0x%x finished, result: action=%s, "
                "param=%d",
                ENUM_MAP_TO_STRING(GF_SEQUENCE_EVENT_TYPE, event->type),
                event->type, event->data,
                ENUM_MAP_TO_STRING(GF_ACTION, gf_cmd.action), gf_cmd.param);
            if (gf_cmd.action != GF_NOOP) {
                return gf_cmd;
            }
        }

        // Update sequence context if necessary
        seq_ctx = GF_SwitchSequenceContext(event, seq_ctx);
    }

    LOG_DEBUG(
        "sequence finished: action=%s param=%d",
        ENUM_MAP_TO_STRING(GF_ACTION, gf_cmd.action), gf_cmd.param);
    return gf_cmd;
}
