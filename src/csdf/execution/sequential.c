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

static void push(void *buffer, const void *token)
{
    CsdfSequentialBuffer *state = buffer;
    size_t tokenSize = state->connection->tokenSize;
    uint8_t *stateTokens = state->tokens;
    void *bufferEnd = stateTokens + tokenSize * state->end;
    memcpy(bufferEnd, token, tokenSize);
    state->end = (state->end + 1) % state->bufferSize;
    if (state->end == state->start)
    {
        exit(-123);
    }
}

static uint8_t *pop(CsdfSequentialBuffer *buffer)
{
    size_t tokenSize = buffer->connection->tokenSize;
    uint8_t *bufferStart = buffer->tokens + tokenSize * buffer->start;
    buffer->start = (buffer->start + 1) % buffer->bufferSize;
    return bufferStart;
}

static unsigned number_tokens(CsdfSequentialBuffer *buffer)
{
    const CsdfSequentialBuffer *state = buffer;
    unsigned end = state->end;
    unsigned start = state->start;

    return end >= start
               ? end - start
               : end + state->bufferSize - start;
}

static size_t calculate_buffer_size(CsdfSequentialRun *runData, const CsdfConnection *buffer)
{
    size_t numInitialTokens = buffer->numTokens;
    size_t actorId = buffer->source.actorId;
    const CsdfActor *actor = runData->graph->actors + actorId;
    const CsdfOutput *output = actor->outputs + buffer->source.outputId;
    size_t potentiallyProducedTokens = runData->repetitionVector[actorId] * output->production;
    return numInitialTokens + potentiallyProducedTokens + 1;
}

static CsdfSequentialBuffer *new_buffer_states(CsdfSequentialRun *runData)
{
    const CsdfGraph *graph = runData->graph;
    CsdfSequentialBuffer *bufferStates = malloc(graph->numConnections * sizeof(CsdfSequentialBuffer));
    for (size_t bufferId = 0; bufferId < graph->numConnections; bufferId++)
    {
        const CsdfConnection *connection = graph->connections + bufferId;
        const uint8_t *bufferInitialTokens = connection->initialTokens;
        CsdfSequentialBuffer *bufferState = &bufferStates[bufferId];
        bufferState->connection = connection;
        bufferState->start = 0;
        bufferState->end = 0;
        size_t bufferSize = calculate_buffer_size(runData, connection);
        bufferState->tokens = malloc(bufferSize * connection->tokenSize);
        bufferState->bufferSize = bufferSize;
        memcpy(bufferState->tokens, bufferInitialTokens, connection->numTokens * connection->tokenSize);
        bufferState->end = connection->numTokens;
    }
    return bufferStates;
}

CsdfActorRun *new_actor_run(const CsdfActor *actor, CsdfRecordData *recordData)
{
    CsdfActorRun *actorRun = malloc(sizeof(CsdfActor));
    actorRun->actor = actor;
    size_t sizeConsumedTokens = 0;
    for (size_t inputId = 0; inputId < actor->numInputs; inputId++)
    {
        const CsdfInput *input = &actor->inputs[inputId];
        sizeConsumedTokens += input->consumption * input->tokenSize;
    }
    actorRun->consumed = malloc(sizeConsumedTokens);

    size_t sizeProducedTokens = 0;
    for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
    {
        const CsdfOutput *output = &actor->outputs[outputId];
        sizeProducedTokens += output->production * output->tokenSize;
    }
    actorRun->produced = malloc(sizeProducedTokens);
    actorRun->recordData = recordData;
    actorRun->inputBuffers = malloc(actor->numInputs * sizeof(CsdfSequentialBuffer *));
    actorRun->outputBuffers = malloc(actor->numOutputs * sizeof(CsdfSequentialBuffer *));
    return actorRun;
}

void set_input_buffer(CsdfActorRun *actorRun, size_t inputId, CsdfSequentialBuffer *buffer)
{
    actorRun->inputBuffers[inputId] = buffer;
}

void set_output_buffer(CsdfActorRun *actorRun, size_t outputId, void *buffer)
{
    actorRun->outputBuffers[outputId] = buffer;
}

CsdfSequentialRun *new_sequential_run(const CsdfGraph *graph, unsigned numIterations)
{
    CsdfSequentialRun *runData = malloc(sizeof(CsdfSequentialRun));
    runData->graph = graph;
    unsigned int *repetitionVector = malloc(graph->numActors * sizeof(size_t));
    csdf_repetition_vector(graph, repetitionVector);
    runData->repetitionVector = repetitionVector;
    runData->bufferStates = new_buffer_states(runData);

    runData->actorRuns = malloc(graph->numActors * sizeof(CsdfActorRun *));
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        size_t numFirings = numIterations * repetitionVector[actorId];
        const CsdfActor *actor = graph->actors + actorId;
        CsdfActorRun *actorRun = new_actor_run(actor, new_record_produced(actor, numFirings));
        runData->actorRuns[actorId] = actorRun;
        for (size_t bufferId = 0; bufferId < graph->numConnections; bufferId++)
        {
            const CsdfConnection *connection = graph->connections + bufferId;
            if (connection->destination.actorId == actorId)
            {
                set_input_buffer(actorRun, connection->destination.inputId, runData->bufferStates + bufferId);
            }
            if (connection->source.actorId == actorId)
            {
                set_output_buffer(actorRun, connection->source.outputId, runData->bufferStates + bufferId);
            }
        }
    }
    return runData;
}

void delete_actor_run(CsdfActorRun *runData)
{
    free(runData->consumed);
    free(runData->inputBuffers);
    free(runData->outputBuffers);
}

void delete_sequential_run(CsdfSequentialRun *runData)
{
    for (size_t bufferId = 0; bufferId < runData->graph->numConnections; bufferId++)
    {
        CsdfSequentialBuffer *bufferState = &runData->bufferStates[bufferId];
        free(bufferState->tokens);
    }
    free(runData->bufferStates);
    free(runData->repetitionVector);
    free(runData);
}

bool can_fire(CsdfActorRun *runData)
{

    const CsdfActor *actor = runData->actor;

    for (size_t dstPortId = 0; dstPortId < actor->numInputs; dstPortId++)
    {

        CsdfSequentialBuffer *buffer = runData->inputBuffers[dstPortId];

        const CsdfInput *dstPort = &actor->inputs[dstPortId];

        if (dstPort->consumption > number_tokens(buffer))
        {

            return false;
        }
    }
    return true;
}

static void consume(CsdfActorRun *runData)
{
    uint8_t *consumed = runData->consumed;
    const CsdfActor *actor = runData->actor;
    for (size_t dstPortId = 0; dstPortId < actor->numInputs; dstPortId++)
    {
        CsdfSequentialBuffer *buffer = runData->inputBuffers[dstPortId];
        const CsdfInput *dstPort = actor->inputs + dstPortId;
        for (size_t tokenId = 0; tokenId < dstPort->consumption; tokenId++)
        {
            uint8_t *token = pop(buffer);
            memcpy(consumed, token, dstPort->tokenSize);
            consumed += dstPort->tokenSize;
        }
    }
}

void produce(CsdfActorRun *runData)
{

    const CsdfActor *actor = runData->actor;
    uint8_t *produced = runData->produced;
    uint8_t *producedByPort = produced;
    const CsdfOutput *srcPort = actor->outputs;

    for (
        size_t srcPortId = 0;
        srcPortId < actor->numOutputs;
        producedByPort += srcPort->tokenSize * srcPort->production, srcPortId++, srcPort++)
    {

        uint8_t *producedForBuffer = producedByPort;

        void *buffer = runData->outputBuffers[srcPortId];
        for (size_t tokenId = 0; tokenId < srcPort->production; tokenId++)
        {

            push(buffer, producedForBuffer);

            producedForBuffer += srcPort->tokenSize;
        }

        producedByPort = producedForBuffer;
    }
}

static void record_results(CsdfActorRun *runData)
{
    uint8_t *produced = runData->produced;
    CsdfRecordData *recordData = runData->recordData;
    if (recordData->on_token_produced != NULL)
    {
        recordData->on_token_produced(produced, recordData);
    }
}

void fire(CsdfActorRun *runData)
{

    consume(runData);

    runData->actor->execution(runData->consumed, runData->produced);

    produce(runData);

    record_results(runData);
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
            if (repetitionVector[actorId] > 0 && can_fire(runData->actorRuns[actorId]))
            {
                fire(runData->actorRuns[actorId]);

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
