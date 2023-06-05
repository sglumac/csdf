/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_EXECUTION_GRAPH_H
#define CSDF_EXECUTION_GRAPH_H

#include "buffer.h"
#include "actor.h"

#include <csdf/graph.h>

typedef struct CsdfGraphRun
{
    const CsdfGraph *graph;
    unsigned int *repetitionVector;
    CsdfBuffer **buffers;
    CsdfActorRun **actorRuns;
    unsigned int numIterations;
} CsdfGraphRun;

CsdfGraphRun *new_graph_run(const CsdfGraph *graph, unsigned numIterations);

void delete_graph_run(CsdfGraphRun *runData);

#endif // CSDF_EXECUTION_GRAPH_H