/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_GRAPH_H
#define CSDF_GRAPH_H

#include "actor.h"

#include <stddef.h>

typedef struct CsdfInputId
{
    const size_t actorId;
    const size_t inputId;
} CsdfInputId;

typedef struct CsdfOutputId
{
    const size_t actorId;
    const size_t outputId;
} CsdfOutputId;

typedef struct CsdfConnection
{
    const CsdfOutputId source;
    const CsdfInputId destination;
    const size_t tokenSize;
    const size_t numTokens;
    const void *const initialTokens;
} CsdfConnection;

typedef struct CsdfGraph
{
    const size_t numActors;
    const CsdfActor *const actors;
    const size_t numConnections;
    const CsdfConnection *const connections;
} CsdfGraph;

#endif // CSDF_GRAPH_H
