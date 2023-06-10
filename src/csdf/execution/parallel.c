/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "parallel.h"

#include <stdlib.h>

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

CsdfParallelActorRun *create_parallel_actor_run(const CsdfThreading *threading, CsdfActorRun *actorRun)
{
    CsdfParallelActorRun *parallelActorRun = malloc(sizeof(CsdfParallelActorRun));
    parallelActorRun->threading = threading;
    parallelActorRun->actorRun = actorRun;

    parallelActorRun->threadData = malloc(threading->threadDataSize);

    if (!threading->createThread(parallelActorRun->threadData, run_actor, parallelActorRun))
    {
        delete_parallel_actor_run(parallelActorRun);
        return NULL;
    }
    return parallelActorRun;
}

bool join_parallel_actor_run(CsdfParallelActorRun *parallelActorRun)
{
    const CsdfThreading *threading = parallelActorRun->threading;
    void *threadData = parallelActorRun->threadData;
    return threading->joinThread(threadData);
}

void delete_parallel_actor_run(CsdfParallelActorRun *parallelActorRun)
{
    free(parallelActorRun->threadData);
    free(parallelActorRun);
}

bool parallel_run(const CsdfThreading *threading, CsdfGraphRun *runData)
{
    const CsdfGraph *graph = runData->graph;

    CsdfParallelActorRun **parallelActorRuns = malloc(graph->numActors * sizeof(CsdfParallelActorRun *));

    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        CsdfParallelActorRun *parallelActorRun = create_parallel_actor_run(threading, runData->actorRuns[actorId]);
        if (parallelActorRun == NULL)
        {
            return false;
        }
        parallelActorRuns[actorId] = parallelActorRun;
    };
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        CsdfParallelActorRun *parallelActorRun = parallelActorRuns[actorId];
        if (!join_parallel_actor_run(parallelActorRun))
        {
            return false;
        }
        delete_parallel_actor_run(parallelActorRun);
    }
    free(parallelActorRuns);
    return true;
}
