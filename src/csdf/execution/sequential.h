/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_EXECUTION_SEQUENTIAL_H
#define CSDF_EXECUTION_SEQUENTIAL_H

#include <csdf/graph.h>
#include <csdf/record.h>
#include <csdf/execution/actor.h>
#include <csdf/execution/buffer.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct CsdfSequentialRun
{
    const CsdfGraph *graph;
    unsigned int *repetitionVector;
    CsdfBuffer **buffers;
    CsdfActorRun **actorRuns;
    unsigned int numIterations;
} CsdfSequentialRun;

CsdfSequentialRun *new_sequential_run(const CsdfGraph *graph, unsigned numIterations);

void delete_sequential_run(CsdfSequentialRun *runData);

bool sequential_run(CsdfSequentialRun *runData);

#endif // CSDF_EXECUTION_SEQUENTIAL_H
