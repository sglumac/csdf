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

typedef struct CsdfSequentialBuffer
{
    const CsdfConnection *connection;
    size_t start;
    size_t end;
    size_t bufferSize;
    uint8_t *tokens;
} CsdfSequentialBuffer;

typedef struct CsdfActorRun
{
    const CsdfActor *actor;
    uint8_t *consumed;
    uint8_t *produced;
    CsdfRecordData *recordData;
    CsdfSequentialBuffer **inputBuffers;
    CsdfSequentialBuffer **outputBuffers;
} CsdfActorRun;

typedef struct CsdfSequentialRun
{
    const CsdfGraph *graph;
    unsigned int *repetitionVector;
    CsdfSequentialBuffer *bufferStates;
    CsdfActorRun **actorRuns;
} CsdfSequentialRun;

CsdfSequentialRun *new_sequential_run(const CsdfGraph *graph, unsigned numIterations);

void delete_sequential_run(CsdfSequentialRun *runData);

bool can_fire(CsdfActorRun *runData);

void fire(CsdfActorRun *runData);

bool sequential_iteration(CsdfSequentialRun *runData);

#endif // CSDF_SEQUENTIAL_H
