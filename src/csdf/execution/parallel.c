/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "parallel.h"

#include <threads.h>
#include <stdlib.h>

static int run_actor(void *actorRunAsVoidPtr)
{
    CsdfActorRun *actorRun = actorRunAsVoidPtr;
    struct timespec pauseLength = {.tv_sec = 0, .tv_nsec = 100};
    while (actorRun->fireCount < actorRun->maxFireCount)
    {
        while (!can_fire(actorRun))
        {
            thrd_sleep(&pauseLength, NULL);
        }
        fire(actorRun);
    }
    return 0;
}

bool parallel_run(CsdfGraphRun *runData)
{
    const CsdfGraph *graph = runData->graph;
    thrd_t *threads = malloc(graph->numActors * sizeof(thrd_t));
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        if (thrd_create(threads + actorId, run_actor, runData->actorRuns[actorId]) != thrd_success)
        {
            return false;
        }
    }
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        int result;
        if (thrd_join(threads[actorId], &result) != thrd_success)
        {
            return false;
        }
    }
    return true;
}