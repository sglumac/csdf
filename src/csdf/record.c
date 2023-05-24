/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "record.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct RecordProducedState
{
    uint8_t ***allProduced;
    size_t **ends;
    size_t maxTokens;
} RecordProducedState;

static void store_produced_tokens(const CsdfGraph *graph, void *recordState, size_t actorId, void *produced)
{
    RecordProducedState *recordProducedState = recordState;
    const CsdfActor *actor = graph->actors + actorId;
    uint8_t **producedByTheActor = recordProducedState->allProduced[actorId];
    size_t *endsByTheActor = recordProducedState->ends[actorId];
    uint8_t *producedTokens = produced;
    for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
    {
        const CsdfOutput *output = actor->outputs + outputId;
        uint8_t *producedByTheOutput = producedByTheActor[outputId] + endsByTheActor[outputId];
        size_t outputTokensSize = output->production * output->tokenSize;
        endsByTheActor[outputId] += outputTokensSize;
        memcpy(producedByTheOutput, producedTokens, outputTokensSize);
        producedTokens += outputTokensSize;
    }
}

CsdfRecordOptions *new_record_produced_options(const CsdfGraph *graph, size_t maxTokens)
{
    CsdfRecordOptions *recordOptions = malloc(sizeof(CsdfRecordOptions));
    RecordProducedState *state = malloc(sizeof(RecordProducedState));
    recordOptions->recordState = state;
    state->maxTokens = maxTokens;
    state->allProduced = malloc(graph->numActors * sizeof(uint8_t **));
    state->ends = malloc(graph->numActors * sizeof(size_t *));

    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        const CsdfActor *actor = graph->actors + actorId;
        state->allProduced[actorId] = malloc(actor->numOutputs * sizeof(uint8_t *));
        state->ends[actorId] = malloc(actor->numOutputs * sizeof(size_t));
        for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
        {
            const CsdfOutput *output = actor->outputs + outputId;
            state->allProduced[actorId][outputId] = malloc(maxTokens * output->tokenSize);
            state->ends[actorId][outputId] = 0;
        }
    }

    recordOptions->recordState = state;
    recordOptions->on_token_produced = store_produced_tokens;
    recordOptions->on_token_consumed = NULL;
    return recordOptions;
}

void delete_record_produced_options(const CsdfGraph *graph, CsdfRecordOptions *recordOptions)
{
    RecordProducedState *state = recordOptions->recordState;
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        const CsdfActor *actor = graph->actors + actorId;
        for (size_t outputId = 0; outputId < actor->numOutputs; outputId++)
        {
            free(state->allProduced[actorId][outputId]);
        }
        free(state->allProduced[actorId]);
        free(state->ends[actorId]);
    }
    free(state->allProduced);
    free(state->ends);
    free(recordOptions->recordState);
    free(recordOptions);
}

void *get_recorded_produced_tokens(void *recordState, size_t actorId, size_t outputId)
{
    RecordProducedState *state = recordState;
    return state->allProduced[actorId][outputId];
}
