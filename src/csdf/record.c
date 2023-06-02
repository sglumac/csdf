/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "record.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void store_produced_tokens(const uint8_t *produced, CsdfRecordData *recordData)
{
    const uint8_t *producedTokens = produced;
    if (recordData->executionsRecorded >= recordData->maxExecutions)
    {
        return;
    }
    const CsdfActor *actor = recordData->actor;
    for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
    {
        const CsdfOutput *output = actor->outputs + outputId;
        size_t outputTokensSize = output->production * output->tokenSize;
        size_t resultsOffset = outputTokensSize * recordData->executionsRecorded;
        uint8_t *recordedOutputResults = recordData->recordedResults[outputId] + resultsOffset;
        memcpy(recordedOutputResults, producedTokens, outputTokensSize);
        producedTokens += outputTokensSize;
    }
    recordData->executionsRecorded++;
}

CsdfRecordData *new_record_produced(const CsdfActor *actor, size_t numExecutions)
{
    CsdfRecordData *recordData = malloc(sizeof(CsdfRecordData));
    recordData->actor = actor;
    recordData->recordedResults = malloc(actor->numOutputs * sizeof(uint8_t *));
    recordData->executionsRecorded = 0;
    recordData->maxExecutions = 0;

    for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
    {
        const CsdfOutput *output = actor->outputs + outputId;
        recordData->recordedResults[outputId] = malloc(
            numExecutions * output->production * output->tokenSize);
    }

    recordData->on_token_produced = store_produced_tokens;
    return recordData;
}

void delete_record_produced(CsdfRecordData *recordData)
{
    const CsdfActor *actor = recordData->actor;
    for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
    {
        free(recordData->recordedResults[outputId]);
    }

    free(recordData->recordedResults);
    free(recordData);
}

void *new_record_storage(const CsdfRecordData *recordData, size_t outputId)
{
    const CsdfOutput *output = recordData->actor->outputs + outputId;
    size_t resultsSize = recordData->executionsRecorded * output->production * output->tokenSize;
    return malloc(resultsSize);
}

void delete_record_storage(void *recordStorage)
{
    free(recordStorage);
}

void copy_recorded_tokens(const CsdfRecordData *recordData, size_t outputId, void *recordStorage)
{
    const CsdfOutput *output = recordData->actor->outputs + outputId;
    size_t resultsSize = recordData->executionsRecorded * output->production * output->tokenSize;
    memcpy(
        recordData->recordedResults[outputId],
        recordStorage, resultsSize);
}
