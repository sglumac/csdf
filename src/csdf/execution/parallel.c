/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "parallel.h"

bool parallel_run(CsdfGraphRun *runData)
{
    const CsdfGraph *graph = runData->graph;
    for (size_t actorId = 0; actorId < graph->numActors; actorId++)
    {
        (void)(actorId);
    }
    return false;
}