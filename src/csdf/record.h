/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_RECORD_H
#define CSDF_RECORD_H

#include "graph.h"

#include <stdbool.h>

typedef void (*CsdfOnTokenProduced)(const CsdfGraph *graph, void *recordState, size_t actorId, void *produced);
typedef void (*CsdfOnTokenConsumed)(const CsdfGraph *graph, void *recordState, size_t actorId, void *consumed);

typedef struct CsdfRecordOptions
{
    void *recordState;
    CsdfOnTokenProduced on_token_produced;
    CsdfOnTokenConsumed on_token_consumed;
} CsdfRecordOptions;

CsdfRecordOptions *new_record_produced_options(const CsdfGraph *graph, size_t maxTokens);

void delete_record_produced_options(const CsdfGraph *graph, CsdfRecordOptions *recordOptions);

void *get_recorded_produced_tokens(void *recordState, size_t actorId, size_t outputId);

#endif // CSDF_RECORD_H
