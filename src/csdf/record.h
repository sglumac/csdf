/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_RECORD_H
#define CSDF_RECORD_H

#include "graph.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct CsdfRecordData CsdfRecordData;

typedef void (*CsdfOnTokenProduced)(const uint8_t *produced, CsdfRecordData *recordData);

struct CsdfRecordData
{
    const CsdfActor *actor;
    uint8_t **recordedResults;
    CsdfOnTokenProduced on_token_produced;
    size_t maxExecutions;
    size_t executionsRecorded;
};

CsdfRecordData *new_record_produced(const CsdfActor *actor, size_t numExecutions);

void delete_record_produced(CsdfRecordData *recordData);

void *new_record_storage(const CsdfRecordData *recordData, size_t outputId);

void delete_record_storage(void *recordStorage);

void copy_recorded_tokens(const CsdfRecordData *recordData, size_t outputId, void *recordStorage);

#endif // CSDF_RECORD_H
