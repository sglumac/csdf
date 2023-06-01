/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_SEQUENTIAL_H
#define CSDF_SEQUENTIAL_H

#include <csdf/graph.h>
#include <csdf/record.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct CsdfBufferState
{
    const CsdfBuffer *buffer;
    size_t start;
    size_t end;
    size_t bufferSize;
    uint8_t *tokens;
} CsdfBufferState;

typedef struct CsdfSequentialRun
{
    const CsdfGraph *graph;
    unsigned int *repetitionVector;
    const CsdfRecordOptions *recordOptions;
    CsdfBufferState *bufferStates;
    uint8_t **consumed;
    uint8_t **produced;
} CsdfSequentialRun;

CsdfSequentialRun *new_sequential_run(const CsdfGraph *graph, const CsdfRecordOptions *recordOptions);

void delete_sequential_run(CsdfSequentialRun *runData);

bool can_fire(CsdfSequentialRun *runData, size_t actorId);

void fire(CsdfSequentialRun *runData, size_t actorId);

bool sequential_iteration(CsdfSequentialRun *runData);

#endif // CSDF_SEQUENTIAL_H
