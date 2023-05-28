/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_SEQUENTIAL_H
#define CSDF_SEQUENTIAL_H

#include "graph.h"
#include "record.h"

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

typedef struct CsdfGraphState
{
    const CsdfGraph *graph;
    unsigned int *repetitionVector;
    CsdfBufferState *bufferStates;
    uint8_t **consumed;
    uint8_t **produced;
} CsdfGraphState;

CsdfGraphState *new_sequential_state(const CsdfGraph *graph);

void delete_sequential_state(CsdfGraphState *state);

bool can_fire(CsdfGraphState *state, size_t actorId);

void fire(const CsdfRecordOptions *recordOptions, CsdfGraphState *state, size_t actorId);

bool sequential_iteration(const CsdfRecordOptions *recordOptions, CsdfGraphState *state);

#endif // CSDF_SEQUENTIAL_H
