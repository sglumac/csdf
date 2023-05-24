/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_REPETITION_H
#define CSDF_REPETITION_H

#include "graph.h"

#include <stdbool.h>

bool csdf_repetition_vector(const CsdfGraph *graph, unsigned int *repetitionVector);

#endif // CSDF_REPETITION_H
