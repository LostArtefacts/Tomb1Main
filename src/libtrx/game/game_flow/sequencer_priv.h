#pragma once

extern void GF_PreSequenceHook(void);
extern GF_SEQUENCE_CONTEXT GF_SwitchSequenceContext(
    const GF_SEQUENCE_EVENT *event, GF_SEQUENCE_CONTEXT seq_ctx);
extern bool GF_ShouldSkipSequenceEvent(
    const GF_LEVEL *level, const GF_SEQUENCE_EVENT *event);
extern GF_COMMAND (
    *GF_GetSequenceEventHandler(GF_SEQUENCE_EVENT_TYPE event_type))(
    const GF_LEVEL *, const GF_SEQUENCE_EVENT *, GF_SEQUENCE_CONTEXT, void *);
