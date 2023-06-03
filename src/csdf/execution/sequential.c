/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "sequential.h"
#include "buffer.h"
#include <csdf/repetition.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct CsdfSequentialBufferData
{
    const CsdfConnection *connection;
    size_t start;
    size_t end;
    size_t maxTokens;
    uint8_t *tokens;
} CsdfSequentialBufferData;

static void push(CsdfBuffer *buffer, const uint8_t *token)
{
    CsdfSequentialBufferData *data = buffer->data;
    size_t tokenSize = buffer->connection->tokenSize;
    uint8_t *bufferEnd = data->tokens + tokenSize * data->end;
    memcpy(bufferEnd, token, tokenSize);
    data->end = (data->end + 1) % data->maxTokens;
    if (data->end == data->start)
    {
        exit(-123);
    }
}

static void pop(CsdfBuffer *buffer, uint8_t *token)
{
    CsdfSequentialBufferData *data = buffer->data;
    size_t tokenSize = buffer->connection->tokenSize;
    uint8_t *bufferStart = data->tokens + tokenSize * data->start;
    data->start = (data->start + 1) % data->maxTokens;
    memcpy(token, bufferStart, buffer->connection->tokenSize);
}

static unsigned number_tokens(CsdfBuffer *buffer)
{
    const CsdfSequentialBufferData *data = buffer->data;
    unsigned end = data->end;
    unsigned start = data->start;

    return end >= start
               ? end - start
               : end + data->maxTokens - start;
}

static size_t calculate_buffer_max_tokens(CsdfSequentialRun *runData, const CsdfConnection *connection)
{
    size_t numInitialTokens = connection->numTokens;
    size_t actorId = connection->source.actorId;
    const CsdfActor *actor = runData->graph->actors + actorId;
    const CsdfOutput *output = actor->outputs + connection->source.outputId;
    size_t potentiallyProducedTokens = runData->repetitionVector[actorId] * output->production;
    return numInitialTokens + 2 * potentiallyProducedTokens + 1;
}

static CsdfSequentialBufferData *new_buffer_data(CsdfSequentialRun *runData, const CsdfConnection *connection)
{
    CsdfSequentialBufferData *data = malloc(sizeof(CsdfSequentialBufferData));
    data->start = 0;
    data->end = 0;
    size_t maxTokens = calculate_buffer_max_tokens(runData, connection);
    data->tokens = malloc(maxTokens * connection->tokenSize);
    data->maxTokens = maxTokens;
    memcpy(data->tokens, connection->initialTokens, connection->numTokens * connection->tokenSize);
    data->end = connection->numTokens;
    return data;
}

static void free_buffer_data(void *bufferData)
{
    CsdfSequentialBufferData *data = bufferData;
    free(data->tokens);
    free(data);
}

static CsdfBuffer *new_buffers(CsdfSequentialRun *runData)
{
    const CsdfGraph *graph = runData->graph;
    CsdfBuffer *buffers = malloc(graph->numConnections * sizeof(CsdfBuffer));
    for (size_t bufferId = 0; bufferId < graph->numConnections; bufferId++)
    {
        const CsdfConnection *connection = graph->connections + bufferId;

        CsdfBuffer *buffer = &buffers[bufferId];
        buffer->connection = connection;
        buffer->data = new_buffer_data(runData, connection);
        buffer->pop = pop;
        buffer->push = push;
        buffer->numberOfTokens = number_tokens;
        buffer->freeData = free_buffer_data;
    }
    return buffers;
}

CsdfActorRun *new_actor_run(const CsdfActor *actor, CsdfRecordData *recordData)
{
    CsdfActorRun *actorRun = malloc(sizeof(CsdfActorRun));
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
    actorRun->inputBuffers = malloc(actor->numInputs * sizeof(CsdfBuffer *));
    actorRun->outputBuffers = malloc(actor->numOutputs * sizeof(CsdfBuffer *));
    return actorRun;
}

void set_input_buffer(CsdfActorRun *actorRun, size_t inputId, CsdfBuffer *buffer)
{
    actorRun->inputBuffers[inputId] = buffer;
}

void set_output_buffer(CsdfActorRun *actorRun, size_t outputId, CsdfBuffer *buffer)
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
    runData->buffers = new_buffers(runData);

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
                set_input_buffer(actorRun, connection->destination.inputId, runData->buffers + bufferId);
            }
            if (connection->source.actorId == actorId)
            {
                set_output_buffer(actorRun, connection->source.outputId, runData->buffers + bufferId);
            }
        }
    }
    return runData;
}

void delete_actor_run(CsdfActorRun *runData)
{
    free(runData->produced);
    free(runData->consumed);
    free(runData->inputBuffers);
    free(runData->outputBuffers);
    free(runData);
}

void delete_sequential_run(CsdfSequentialRun *runData)
{
    for (size_t bufferId = 0; bufferId < runData->graph->numConnections; bufferId++)
    {
        CsdfBuffer *buffer = &runData->buffers[bufferId];
        buffer->freeData(buffer->data);
    }
    for (size_t actorId = 0; actorId < runData->graph->numActors; actorId++)
    {
        delete_record_produced(runData->actorRuns[actorId]->recordData);
        delete_actor_run(runData->actorRuns[actorId]);
    }
    free(runData->buffers);
    free(runData->repetitionVector);
    free(runData->actorRuns);
    free(runData);
}

bool can_fire(CsdfActorRun *runData)
{

    const CsdfActor *actor = runData->actor;

    for (size_t dstPortId = 0; dstPortId < actor->numInputs; dstPortId++)
    {

        CsdfBuffer *buffer = runData->inputBuffers[dstPortId];

        const CsdfInput *dstPort = &actor->inputs[dstPortId];

        if (dstPort->consumption > buffer->numberOfTokens(buffer))
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
        CsdfBuffer *buffer = runData->inputBuffers[dstPortId];
        const CsdfInput *dstPort = actor->inputs + dstPortId;
        uint8_t *token = malloc(dstPort->tokenSize);
        for (size_t tokenId = 0; tokenId < dstPort->consumption; tokenId++)
        {
            buffer->pop(buffer, token);
            memcpy(consumed, token, dstPort->tokenSize);
            consumed += dstPort->tokenSize;
        }
        free(token);
    }
}

void produce(CsdfActorRun *runData)
{

    const CsdfActor *actor = runData->actor;
    uint8_t *produced = runData->produced;
    uint8_t *token = produced;
    const CsdfOutput *output = actor->outputs;

    for (
        size_t outputId = 0;
        outputId < actor->numOutputs;
        outputId++, output++)
    {
        CsdfBuffer *buffer = runData->outputBuffers[outputId];
        for (size_t tokenId = 0; tokenId < output->production; tokenId++)
        {
            buffer->push(buffer, token);

            token += output->tokenSize;
        }
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
