/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_ACTOR_H
#define CSDF_ACTOR_H

#include <stddef.h>

typedef void (*ActorExecution)(const void *consumed, void *produced);

typedef struct CsdfInput
{
    const size_t tokenSize;
    const unsigned int consumption;
    char _pad[4];
} CsdfInput;

typedef struct CsdfOutput
{
    const size_t tokenSize;
    const unsigned int production;
    char _pad[4];
} CsdfOutput;

typedef struct CsdfActor
{
    const ActorExecution execution;
    const size_t numInputs;
    const CsdfInput *const inputs;
    const size_t numOutputs;
    const CsdfOutput *const outputs;
} CsdfActor;

#define CSDF_INPUT(type, rate)    \
    {                             \
        .consumption = rate,      \
        .tokenSize = sizeof(type) \
    }

#define CSDF_OUTPUT(type, rate)   \
    {                             \
        .production = rate,       \
        .tokenSize = sizeof(type) \
    }

#endif // CSDF_ACTOR_H
