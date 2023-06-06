/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_EXECUTION_PARALLEL_H
#define CSDF_EXECUTION_PARALLEL_H

#include "graph.h"

#include <threading4csdf.h>

bool parallel_run(const CsdfThreading *parallelActorRun, CsdfGraphRun *runData);

#endif // CSDF_EXECUTION_PARALLEL_H
