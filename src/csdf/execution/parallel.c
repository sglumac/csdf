/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "parallel.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

static void *run_actor(void *actorRunAsVoidPtr)
{
    CsdfActorRun *actorRun = actorRunAsVoidPtr;
    while (actorRun->fireCount < actorRun->maxFireCount)
    {
        while (!can_fire(actorRun))
        {
            usleep(100);
        }
        fire(actorRun);
    }
    pthread_exit(0);
}

bool parallel_run(CsdfGraphRun *runData)
{
    const CsdfGraph *graph = runData->graph;
    pthread_t *threads = malloc(graph->numActors * sizeof(pthread_t));
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        if (pthread_create(threads + actorId, NULL, run_actor, runData->actorRuns[actorId]))
        {
            return false;
        }
    }
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        if (pthread_join(threads[actorId], NULL))
        {
            return false;
        }
    }
    return true;
}