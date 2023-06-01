/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "sequential.h"
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

static size_t calculate_buffer_size(CsdfSequentialRun *runData, const CsdfBuffer *buffer)
{
    size_t numInitialTokens = buffer->numTokens;
    size_t actorId = buffer->source.actorId;
    const CsdfActor *actor = runData->graph->actors + actorId;
    const CsdfOutput *output = actor->outputs + buffer->source.outputId;
    size_t potentiallyProducedTokens = runData->repetitionVector[actorId] * output->production;
    return numInitialTokens + potentiallyProducedTokens + 1;
}

static CsdfBufferState *new_buffer_states(CsdfSequentialRun *runData)
{
    const CsdfGraph *graph = runData->graph;
    CsdfBufferState *bufferStates = malloc(graph->numBuffers * sizeof(CsdfBufferState));
    for (size_t bufferId = 0; bufferId < graph->numBuffers; bufferId++)
    {
        const CsdfBuffer *buffer = &graph->buffers[bufferId];
        const uint8_t *bufferInitialTokens = buffer->initialTokens;
        CsdfBufferState *bufferState = &bufferStates[bufferId];
        bufferState->buffer = buffer;
        bufferState->start = 0;
        bufferState->end = 0;
        size_t bufferSize = calculate_buffer_size(runData, buffer);
        bufferState->tokens = malloc(bufferSize * buffer->tokenSize);
        bufferState->bufferSize = bufferSize;
        uint8_t *bufferStateTokens = bufferState->tokens;
        memcpy(bufferStateTokens, bufferInitialTokens, buffer->numTokens * buffer->tokenSize);
        bufferState->end = buffer->numTokens;
    }
    return bufferStates;
}

CsdfSequentialRun *new_sequential_run(const CsdfGraph *graph, const CsdfRecordOptions *recordOptions)
{
    CsdfSequentialRun *runData = malloc(sizeof(CsdfSequentialRun));
    runData->graph = graph;
    runData->recordOptions = recordOptions;
    unsigned int *repetitionVector = malloc(graph->numActors * sizeof(size_t));
    csdf_repetition_vector(graph, repetitionVector);
    runData->repetitionVector = repetitionVector;
    runData->bufferStates = new_buffer_states(runData);

    runData->consumed = malloc(graph->numActors * sizeof(uint8_t *));
    runData->produced = malloc(graph->numActors * sizeof(uint8_t *));
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        const CsdfActor *actor = &graph->actors[actorId];
        size_t sizeConsumedTokens = 0;
        for (size_t inputId = 0; inputId < actor->numInputs; inputId++)
        {
            const CsdfInput *input = &actor->inputs[inputId];
            sizeConsumedTokens += input->consumption * input->tokenSize;
        }
        runData->consumed[actorId] = malloc(sizeConsumedTokens);

        size_t sizeProducedTokens = 0;
        for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
        {
            const CsdfOutput *output = &actor->outputs[outputId];
            sizeProducedTokens += output->production * output->tokenSize;
        }
        runData->produced[actorId] = malloc(sizeProducedTokens);
    }
    return runData;
}

void delete_sequential_run(CsdfSequentialRun *runData)
{
    for (size_t actorId = 0; actorId < runData->graph->numActors; actorId++)
    {
        free(runData->consumed[actorId]);
        free(runData->produced[actorId]);
    }
    for (size_t bufferId = 0; bufferId < runData->graph->numBuffers; bufferId++)
    {
        CsdfBufferState *bufferState = &runData->bufferStates[bufferId];
        free(bufferState->tokens);
    }
    free(runData->consumed);
    free(runData->produced);
    free(runData->bufferStates);
    free(runData->repetitionVector);
    free(runData);
}

bool can_fire(CsdfSequentialRun *runData, size_t actorId)
{
    const CsdfActor *actor = &runData->graph->actors[actorId];

    for (size_t bufferId = 0; bufferId < runData->graph->numBuffers; bufferId++)
    {
        const CsdfBufferState *bufferState = &runData->bufferStates[bufferId];
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

static uint8_t *consume(CsdfSequentialRun *runData, size_t actorId)
{
    uint8_t *consumed = runData->consumed[actorId];
    const CsdfActor *actor = &runData->graph->actors[actorId];
    for (size_t dstPortId = 0; dstPortId < actor->numInputs; dstPortId++)
    {
        for (size_t bufferId = 0; bufferId < runData->graph->numBuffers; bufferId++)
        {
            CsdfBufferState *bufferState = &runData->bufferStates[bufferId];
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
    return runData->consumed[actorId];
}

void produce(CsdfSequentialRun *runData, size_t actorId)
{
    const CsdfActor *actor = &runData->graph->actors[actorId];
    uint8_t *produced = runData->produced[actorId];
    uint8_t *producedByPort = produced;
    const CsdfOutput *srcPort = actor->outputs;
    for (
        size_t srcPortId = 0;
        srcPortId < actor->numOutputs;
        producedByPort += srcPort->tokenSize * srcPort->production, srcPortId++, srcPort++)
    {
        uint8_t *producedForBuffer = producedByPort;
        for (size_t bufferId = 0; bufferId < runData->graph->numBuffers; bufferId++, producedForBuffer = producedByPort)
        {
            CsdfBufferState *bufferState = &runData->bufferStates[bufferId];
            if (bufferState->buffer->source.actorId == actorId && bufferState->buffer->source.outputId == srcPortId)
            {
                for (size_t tokenId = 0; tokenId < srcPort->production; tokenId++)
                {
                    push(bufferState, producedForBuffer);
                    producedForBuffer += srcPort->tokenSize;
                }
            }
        }
        producedByPort = producedForBuffer;
    }
}

static void record_results(const CsdfRecordOptions *recordOptions, CsdfSequentialRun *runData, size_t actorId)
{
    uint8_t *produced = runData->produced[actorId];

    if (recordOptions->on_token_produced != NULL)
    {
        recordOptions->on_token_produced(runData->graph, recordOptions->recordState, actorId, produced);
    }
}

void fire(CsdfSequentialRun *runData, size_t actorId)
{
    uint8_t *consumed = consume(runData, actorId);
    uint8_t *produced = runData->produced[actorId];
    const CsdfActor *actor = &runData->graph->actors[actorId];
    actor->execution(consumed, produced);
    produce(runData, actorId);
    record_results(runData->recordOptions, runData, actorId);
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

bool sequential_iteration(CsdfSequentialRun *runData)
{
    bool blocked = false;
    unsigned int numActors = runData->graph->numActors;
    unsigned int *repetitionVector = malloc(numActors * sizeof(unsigned int));
    memcpy(repetitionVector, runData->repetitionVector, numActors * sizeof(unsigned int));
    while (!blocked)
    {
        blocked = true;
        for (size_t actorId = 0; actorId < numActors; actorId++)
        {
            if (repetitionVector[actorId] > 0 && can_fire(runData, actorId))
            {
                fire(runData, actorId);
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
