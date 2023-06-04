/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "sequential.h"
#include "buffer/stdlockfree.h"
#include "actor.h"
#include <csdf/repetition.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static size_t calculate_buffer_max_tokens(CsdfSequentialRun *runData, const CsdfConnection *connection)
{
    size_t numInitialTokens = connection->numTokens;
    size_t actorId = connection->source.actorId;
    const CsdfActor *actor = runData->graph->actors + actorId;
    const CsdfOutput *output = actor->outputs + connection->source.outputId;
    size_t potentiallyProducedTokens = runData->repetitionVector[actorId] * output->production;
    return numInitialTokens + 2 * potentiallyProducedTokens + 1;
}

static void create_buffers(CsdfSequentialRun *runData)
{
    const CsdfGraph *graph = runData->graph;
    runData->buffers = malloc(graph->numConnections * sizeof(CsdfBuffer *));
    for (size_t bufferId = 0; bufferId < graph->numConnections; bufferId++)
    {
        const CsdfConnection *connection = graph->connections + bufferId;

        runData->buffers[bufferId] = new_stdlockfree_buffer(connection, calculate_buffer_max_tokens(runData, connection));
    }
}

static void create_actor_runs(CsdfSequentialRun *runData, unsigned numIterations)
{
    const CsdfGraph *graph = runData->graph;
    runData->actorRuns = malloc(graph->numActors * sizeof(CsdfActorRun *));
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        const CsdfActor *actor = graph->actors + actorId;

        size_t numFirings = numIterations * runData->repetitionVector[actorId];
        CsdfRecordData *recordData = new_record_produced(actor, numFirings);

        CsdfBuffer **inputBuffers = malloc(actor->numInputs * sizeof(CsdfBuffer *));
        CsdfBuffer **outputBuffers = malloc(actor->numOutputs * sizeof(CsdfBuffer *));

        for (size_t bufferId = 0; bufferId < graph->numConnections; bufferId++)
        {
            const CsdfConnection *connection = graph->connections + bufferId;
            if (connection->destination.actorId == actorId)
            {
                inputBuffers[connection->destination.inputId] = runData->buffers[bufferId];
            }
            if (connection->source.actorId == actorId)
            {
                outputBuffers[connection->source.outputId] = runData->buffers[bufferId];
            }
        }

        runData->actorRuns[actorId] = new_actor_run(actor, recordData, inputBuffers, outputBuffers);
    }
}

CsdfSequentialRun *new_sequential_run(const CsdfGraph *graph, unsigned numIterations)
{
    CsdfSequentialRun *runData = malloc(sizeof(CsdfSequentialRun));
    runData->graph = graph;
    unsigned int *repetitionVector = malloc(graph->numActors * sizeof(size_t));
    csdf_repetition_vector(graph, repetitionVector);
    runData->repetitionVector = repetitionVector;
    create_buffers(runData);
    create_actor_runs(runData, numIterations);
    return runData;
}

void delete_sequential_run(CsdfSequentialRun *runData)
{
    for (size_t bufferId = 0; bufferId < runData->graph->numConnections; bufferId++)
    {
        delete_stdlockfree_buffer(runData->buffers[bufferId]);
    }
    for (size_t actorId = 0; actorId < runData->graph->numActors; actorId++)
    {
        CsdfActorRun *actorRun = runData->actorRuns[actorId];
        delete_record_produced(actorRun->recordData);
        free(actorRun->inputBuffers);
        free(actorRun->outputBuffers);
        delete_actor_run(actorRun);
    }
    free(runData->buffers);
    free(runData->repetitionVector);
    free(runData->actorRuns);
    free(runData);
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
