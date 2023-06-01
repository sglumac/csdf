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

typedef struct CsdfRecordOptions
{
    void *recordState;
    CsdfOnTokenProduced on_token_produced;
} CsdfRecordOptions;

CsdfRecordOptions *new_record_produced_options(const CsdfGraph *graph, size_t maxTokens);

void delete_record_produced_options(const CsdfGraph *graph, CsdfRecordOptions *recordOptions);

void *new_record_storage(const CsdfRecordOptions *recordOptions, size_t actorId, size_t outputId);

void delete_record_storage(void *recordStorage);

void copy_recorded_produced_tokens(const CsdfRecordOptions *recordOptions, size_t actorId, size_t outputId, void *recordStorage);

#endif // CSDF_RECORD_H
