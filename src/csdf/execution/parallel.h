/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_EXECUTION_PARALLEL_H
#define CSDF_EXECUTION_PARALLEL_H

#include "graph.h"

typedef bool (*CsdfTask)(void *taskData);
typedef void (*CsdfSleep)(unsigned int microseconds);
typedef bool (*CsdfCreateThread)(void *threadData, CsdfTask task, void *taskData);
typedef bool (*CsdfThreadJoin)(void *threadData);

typedef struct CsdfThreading
{
    size_t threadDataSize;
    CsdfSleep sleep;
    CsdfCreateThread createThread;
    CsdfThreadJoin joinThread;
    unsigned int microsecondsSleep;
} CsdfThreading;

bool parallel_run(const CsdfThreading *parallelActorRun, CsdfGraphRun *runData);

#endif // CSDF_EXECUTION_PARALLEL_H
