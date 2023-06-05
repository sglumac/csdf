/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "graph.h"
#include "buffer/stdlockfree.h"

#include <csdf/repetition.h>

#include <stdlib.h>

static size_t calculate_buffer_max_tokens(CsdfGraphRun *runData, const CsdfConnection *connection)
{
    size_t numInitialTokens = connection->numTokens;
    size_t actorId = connection->source.actorId;
    const CsdfActor *actor = runData->graph->actors + actorId;
    const CsdfOutput *output = actor->outputs + connection->source.outputId;
    size_t potentiallyProducedTokens = runData->repetitionVector[actorId] * output->production;
    return numInitialTokens + 2 * potentiallyProducedTokens + 1;
}

static void create_buffers(CsdfGraphRun *runData)
{
    const CsdfGraph *graph = runData->graph;
    runData->buffers = malloc(graph->numConnections * sizeof(CsdfBuffer *));
    for (size_t bufferId = 0; bufferId < graph->numConnections; bufferId++)
    {
        const CsdfConnection *connection = graph->connections + bufferId;

        runData->buffers[bufferId] = new_stdlockfree_buffer(connection, calculate_buffer_max_tokens(runData, connection));
    }
}

static void create_actor_runs(CsdfGraphRun *runData, unsigned numIterations)
{
    const CsdfGraph *graph = runData->graph;
    runData->actorRuns = malloc(graph->numActors * sizeof(CsdfActorRun *));
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        const CsdfActor *actor = graph->actors + actorId;

        size_t numFirings = numIterations * runData->repetitionVector[actorId];
        CsdfRecordData *recordData = new_record_produced(actor, numFirings);

        CsdfBuffer **inputBuffers = malloc(actor->numInputs * sizeof(CsdfBuffer *));
        CsdfBuffer ***outputBuffers = malloc(actor->numOutputs * sizeof(CsdfBuffer **));

        size_t *numOutputBuffers = calloc(actor->numOutputs, sizeof(size_t));
        for (size_t bufferId = 0; bufferId < graph->numConnections; bufferId++)
        {
            const CsdfConnection *connection = graph->connections + bufferId;
            if (connection->destination.actorId == actorId)
            {
                inputBuffers[connection->destination.inputId] = runData->buffers[bufferId];
            }
            if (connection->source.actorId == actorId)
            {
                numOutputBuffers[connection->source.outputId]++;
            }
        }
        for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
        {
            outputBuffers[outputId] = malloc(numOutputBuffers[outputId] * sizeof(CsdfBuffer *));
        }
        size_t *bufferOutputIds = calloc(actor->numOutputs, sizeof(size_t));
        for (size_t bufferId = 0; bufferId < graph->numConnections; bufferId++)
        {
            const CsdfConnection *connection = graph->connections + bufferId;
            if (connection->source.actorId == actorId)
            {
                size_t outputId = connection->source.outputId;
                size_t outputBufferId = bufferOutputIds[outputId]++;
                outputBuffers[outputId][outputBufferId] = runData->buffers[bufferId];
            }
        }
        free(bufferOutputIds);

        runData->actorRuns[actorId] = new_actor_run(actor, recordData, inputBuffers, outputBuffers, numOutputBuffers);
    }
    runData->numIterations = numIterations;
}

CsdfGraphRun *new_sequential_run(const CsdfGraph *graph, unsigned numIterations)
{
    CsdfGraphRun *runData = malloc(sizeof(CsdfGraphRun));
    runData->graph = graph;
    unsigned int *repetitionVector = malloc(graph->numActors * sizeof(size_t));
    csdf_repetition_vector(graph, repetitionVector);
    runData->repetitionVector = repetitionVector;
    create_buffers(runData);
    create_actor_runs(runData, numIterations);
    return runData;
}

void delete_sequential_run(CsdfGraphRun *runData)
{
    for (size_t bufferId = 0; bufferId < runData->graph->numConnections; bufferId++)
    {
        delete_stdlockfree_buffer(runData->buffers[bufferId]);
    }
    for (size_t actorId = 0; actorId < runData->graph->numActors; actorId++)
    {
        CsdfActorRun *actorRun = runData->actorRuns[actorId];
        for (size_t outputId = 0; outputId < actorRun->actor->numOutputs; outputId++)
        {
            free(actorRun->outputBuffers[outputId]);
        }
        delete_record_produced(actorRun->recordData);
        free(actorRun->inputBuffers);
        free(actorRun->outputBuffers);
        free(actorRun->numOutputBuffers);
        delete_actor_run(actorRun);
    }
    free(runData->buffers);
    free(runData->repetitionVector);
    free(runData->actorRuns);
    free(runData);
}
