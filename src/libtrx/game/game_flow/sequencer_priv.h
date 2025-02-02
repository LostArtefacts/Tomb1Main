#pragma once

extern void GF_PreSequenceHook(void);
extern GF_SEQUENCE_CONTEXT GF_SwitchSequenceContext(
    const GF_SEQUENCE_EVENT *event, GF_SEQUENCE_CONTEXT seq_ctx);
extern bool GF_ShouldSkipSequenceEvent(
    const GF_LEVEL *level, const GF_SEQUENCE_EVENT *event);

// Defer execution of certain events to run it at various stages of
// GFS_PLAY_LEVEL.
extern GF_EVENT_QUEUE_TYPE GF_ShouldDeferSequenceEvent(
    GF_SEQUENCE_EVENT_TYPE event_type);

extern GF_SEQUENCE_EVENT_HANDLER GF_GetSequenceEventHandler(
    GF_SEQUENCE_EVENT_TYPE event_type);
