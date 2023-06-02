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

typedef unsigned int (*NumberTokensGetter)(void *buffer);

typedef struct CsdfActorRun
{
    CsdfActor *actor;
    size_t actorId;
    uint8_t *consumed;
    uint8_t *produced;
    CsdfRecordData *recordData;
    void **inputBuffers;
    void **outputBuffers;
    NumberTokensGetter get_number_tokens;
} CsdfActorRun;

typedef struct CsdfSequentialRun
{
    const CsdfGraph *graph;
    unsigned int *repetitionVector;
    CsdfBufferState *bufferStates;
    CsdfActorRun **actorRuns;
    CsdfRecordData *recordData;
} CsdfSequentialRun;

CsdfSequentialRun *new_sequential_run(const CsdfGraph *graph, unsigned numIterations);

void delete_sequential_run(CsdfSequentialRun *runData);

bool can_fire(CsdfActorRun *runData);

void fire(CsdfActorRun *runData);

bool sequential_iteration(CsdfSequentialRun *runData);

#endif // CSDF_SEQUENTIAL_H
