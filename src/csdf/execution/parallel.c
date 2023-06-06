/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "parallel.h"

#include <stdlib.h>

typedef struct CsdfParallelActorRun
{
    const CsdfThreading *threading;
    CsdfActorRun *actorRun;
} CsdfParallelActorRun;

static bool run_actor(void *taskData)
{
    CsdfParallelActorRun *parallel = taskData;
    CsdfActorRun *actorRun = parallel->actorRun;
    const CsdfThreading *threading = parallel->threading;

    while (actorRun->fireCount < actorRun->maxFireCount)
    {
        while (!can_fire(actorRun))
        {
            threading->sleep(threading->microsecondsSleep);
        }
        fire(actorRun);
    }
    return true;
}

bool parallel_run(const CsdfThreading *threading, CsdfGraphRun *runData)
{
    const CsdfGraph *graph = runData->graph;

    CsdfParallelActorRun *parallelActorRuns = malloc(graph->numActors * sizeof(CsdfParallelActorRun));

    void **threadsData = malloc(graph->numActors * sizeof(void *));

    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        CsdfParallelActorRun *parallelActorRun = parallelActorRuns + actorId;
        parallelActorRun->threading = threading;
        parallelActorRun->actorRun = runData->actorRuns[actorId];

        threadsData[actorId] = malloc(threading->threadDataSize);

        if (!threading->createThread(threadsData[actorId], run_actor, parallelActorRun))
        {
            return false;
        }
    };
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        if (!threading->joinThread(threadsData[actorId]))
        {
            return false;
        }
        free(threadsData[actorId]);
    }
    free(threadsData);
    free(parallelActorRuns);
    return true;
}
