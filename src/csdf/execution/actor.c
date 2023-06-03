/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "actor.h"

#include <stdlib.h>
#include <string.h>

static void record_results(CsdfActorRun *runData)
{
    uint8_t *produced = runData->produced;
    CsdfRecordData *recordData = runData->recordData;
    if (recordData->on_token_produced != NULL)
    {
        recordData->on_token_produced(produced, recordData);
    }
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

static void produce(CsdfActorRun *runData)
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

void fire(CsdfActorRun *runData)
{

    consume(runData);

    runData->actor->execution(runData->consumed, runData->produced);

    produce(runData);

    record_results(runData);
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

void delete_actor_run(CsdfActorRun *runData)
{
    free(runData->produced);
    free(runData->consumed);
    free(runData->inputBuffers);
    free(runData->outputBuffers);
    free(runData);
}
