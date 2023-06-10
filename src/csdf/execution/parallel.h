/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_EXECUTION_PARALLEL_H
#define CSDF_EXECUTION_PARALLEL_H

#include "graphrun.h"

#include <threading4csdf.h>

typedef struct CsdfParallelActorRun
{
    const CsdfThreading *threading;
    CsdfActorRun *actorRun;
    void *threadData;
} CsdfParallelActorRun;

bool parallel_run(const CsdfThreading *threading, CsdfGraphRun *runData);

CsdfParallelActorRun *create_parallel_actor_run(const CsdfThreading *threading, CsdfActorRun *actorRun);

bool join_parallel_actor_run(CsdfParallelActorRun *parallelActorRun);

void delete_parallel_actor_run(CsdfParallelActorRun *parallelActorRun);

#endif // CSDF_EXECUTION_PARALLEL_H
