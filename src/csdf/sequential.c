/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include <csdf/sequential.h>
#include <csdf/repetition.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static void push(CsdfBufferState *state, const void *token)
{
    size_t tokenSize = state->buffer->tokenSize;
    uint8_t *stateTokens = state->tokens;
    void *bufferEnd = stateTokens + tokenSize * state->end;
    memcpy(bufferEnd, token, tokenSize);
    state->end = (state->end + 1) % state->bufferSize;
    if (state->end == state->start)
    {
        exit(-123);
    }
}

static uint8_t *pop(CsdfBufferState *state)
{
    size_t tokenSize = state->buffer->tokenSize;
    uint8_t *bufferStart = state->tokens + tokenSize * state->start;
    state->start = (state->start + 1) % state->bufferSize;
    return bufferStart;
}

static unsigned number_tokens(const CsdfBufferState *state)
{
    unsigned end = state->end;
    unsigned start = state->start;

    return end >= start
               ? end - start
               : end + state->bufferSize - start;
}

static CsdfBufferState *new_buffer_states(const CsdfGraph *graph, size_t bufferSize)
{
    CsdfBufferState *bufferStates = malloc(graph->numBuffers * sizeof(CsdfBufferState));
    for (size_t bufferId = 0; bufferId < graph->numBuffers; bufferId++)
    {
        const CsdfBuffer *buffer = &graph->buffers[bufferId];
        const uint8_t *bufferInitialTokens = buffer->initialTokens;
        CsdfBufferState *bufferState = &bufferStates[bufferId];
        bufferState->buffer = buffer;
        bufferState->start = 0;
        bufferState->end = 0;
        bufferState->tokens = malloc(bufferSize * buffer->tokenSize);
        bufferState->bufferSize = bufferSize;
        uint8_t *bufferStateTokens = bufferState->tokens;
        memcpy(bufferStateTokens, bufferInitialTokens, buffer->numTokens * buffer->tokenSize);
        bufferState->end = buffer->numTokens;
    }
    return bufferStates;
}

CsdfGraphState *new_sequential_state(const CsdfGraph *graph, int maxBufferTokens)
{
    CsdfGraphState *state = malloc(sizeof(CsdfGraphState));
    state->graph = graph;
    state->bufferStates = new_buffer_states(graph, maxBufferTokens);

    state->consumed = malloc(graph->numActors * sizeof(uint8_t *));
    state->produced = malloc(graph->numActors * sizeof(uint8_t *));
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        const CsdfActor *actor = &graph->actors[actorId];
        size_t sizeConsumedTokens = 0;
        for (size_t inputId = 0; inputId < actor->numInputs; inputId++)
        {
            const CsdfInput *input = &actor->inputs[inputId];
            sizeConsumedTokens += input->consumption * input->tokenSize;
        }
        state->consumed[actorId] = malloc(sizeConsumedTokens);

        size_t sizeProducedTokens = 0;
        for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
        {
            const CsdfOutput *output = &actor->outputs[outputId];
            sizeProducedTokens += output->production * output->tokenSize;
        }
        state->produced[actorId] = malloc(sizeProducedTokens);
    }
    unsigned int *repetitionVector = malloc(graph->numActors * sizeof(size_t));
    csdf_repetition_vector(graph, repetitionVector);
    state->repetitionVector = repetitionVector;
    return state;
}

void delete_sequential_state(CsdfGraphState *state)
{
    for (size_t actorId = 0; actorId < state->graph->numActors; actorId++)
    {
        free(state->consumed[actorId]);
        free(state->produced[actorId]);
    }
    for (size_t bufferId = 0; bufferId < state->graph->numBuffers; bufferId++)
    {
        CsdfBufferState *bufferState = &state->bufferStates[bufferId];
        free(bufferState->tokens);
    }
    free(state->consumed);
    free(state->produced);
    free(state->bufferStates);
    free(state->repetitionVector);
    free(state);
}

bool can_fire(CsdfGraphState *state, size_t actorId)
{
    const CsdfActor *actor = &state->graph->actors[actorId];

    for (size_t bufferId = 0; bufferId < state->graph->numBuffers; bufferId++)
    {
        const CsdfBufferState *bufferState = &state->bufferStates[bufferId];
        if (bufferState->buffer->destination.actorId == actorId)
        {
            size_t dstPortId = bufferState->buffer->destination.inputId;
            const CsdfInput *dstPort = &actor->inputs[dstPortId];
            if (dstPort->consumption > number_tokens(bufferState))
            {
                return false;
            }
        }
    }
    return true;
}

static uint8_t *consume(CsdfGraphState *state, size_t actorId)
{
    uint8_t *consumed = state->consumed[actorId];
    const CsdfActor *actor = &state->graph->actors[actorId];
    for (size_t dstPortId = 0; dstPortId < actor->numInputs; dstPortId++)
    {
        for (size_t bufferId = 0; bufferId < state->graph->numBuffers; bufferId++)
        {
            CsdfBufferState *bufferState = &state->bufferStates[bufferId];
            const CsdfInputId *dstPort = &bufferState->buffer->destination;
            if (dstPort->actorId == actorId && dstPort->inputId == dstPortId)
            {
                const CsdfInput *dstPort = &actor->inputs[dstPortId];
                for (size_t tokenId = 0; tokenId < dstPort->consumption; tokenId++)
                {
                    uint8_t *token = pop(bufferState);
                    memcpy(consumed, token, dstPort->tokenSize);
                    consumed += dstPort->tokenSize;
                }
            }
        }
    }
    return state->consumed[actorId];
}

void produce(CsdfGraphState *state, size_t actorId)
{
    const CsdfActor *actor = &state->graph->actors[actorId];
    uint8_t *produced = state->produced[actorId];
    for (size_t bufferId = 0; bufferId < state->graph->numBuffers; bufferId++)
    {
        CsdfBufferState *bufferState = &state->bufferStates[bufferId];
        if (bufferState->buffer->source.actorId == actorId)
        {
            size_t srcPortId = bufferState->buffer->source.outputId;
            const CsdfOutput *srcPort = &actor->outputs[srcPortId];
            for (size_t tokenId = 0; tokenId < srcPort->production; tokenId++)
            {
                push(bufferState, produced);
                produced += srcPort->tokenSize;
            }
        }
    }
}

static void record_results(const CsdfRecordOptions *recordOptions, CsdfGraphState *state, size_t actorId)
{
    uint8_t *produced = state->produced[actorId];
    uint8_t *consumed = state->consumed[actorId];

    if (recordOptions->on_token_consumed != NULL)
    {
        recordOptions->on_token_consumed(state->graph, recordOptions->recordState, actorId, consumed);
    }
    if (recordOptions->on_token_produced != NULL)
    {
        recordOptions->on_token_produced(state->graph, recordOptions->recordState, actorId, produced);
    }
}

void fire(const CsdfRecordOptions *recordOptions, CsdfGraphState *state, size_t actorId)
{
    uint8_t *consumed = consume(state, actorId);
    uint8_t *produced = state->produced[actorId];
    const CsdfActor *actor = &state->graph->actors[actorId];
    actor->execution(consumed, produced);
    produce(state, actorId);
    record_results(recordOptions, state, actorId);
}

static bool all_zero(unsigned int *repetitionVector, size_t numActors)
{
    for (size_t actorId = 0; actorId < numActors; actorId++)
    {
        if (repetitionVector[actorId] > 0)
        {
            return false;
        }
    }
    return true;
}

bool sequential_iteration(const CsdfRecordOptions *recordOptions, CsdfGraphState *state)
{
    bool blocked = false;
    unsigned int numActors = state->graph->numActors;
    unsigned int *repetitionVector = malloc(numActors * sizeof(unsigned int));
    memcpy(repetitionVector, state->repetitionVector, numActors * sizeof(unsigned int));
    while (!blocked)
    {
        blocked = true;
        for (size_t actorId = 0; actorId < numActors; actorId++)
        {
            if (repetitionVector[actorId] > 0 && can_fire(state, actorId))
            {
                fire(recordOptions, state, actorId);
                repetitionVector[actorId]--;
                blocked = false;
                break;
            }
        }
    }
    bool iterationCompleted = all_zero(repetitionVector, numActors);
    free(repetitionVector);
    return iterationCompleted;
}
