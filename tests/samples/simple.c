/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "simple.h"

#define UNUSED(x) (void)(x)

static void three_execute(const void *consumed, void *produced)
{
    UNUSED(consumed);
    double *y = produced;
    *y = 3;
}

CsdfInput doubleInput[] = {CSDF_INPUT(double, 1)};

CsdfOutput doubleOutput[] = {CSDF_OUTPUT(double, 1)};

#define THREE_CONSTANT              \
    {                               \
        .execution = three_execute, \
        .numInputs = 0,             \
        .inputs = NULL,             \
        .numOutputs = 1,            \
        .outputs = doubleOutput     \
    }

static void
double_execute(const void *consumed, void *produced)
{
    const double *u = consumed;
    double *y = produced;
    *y = *u * 2;
}

#define DOUBLE_GAIN                  \
    {                                \
        .execution = double_execute, \
        .numInputs = 1,              \
        .inputs = doubleInput,       \
        .numOutputs = 1,             \
        .outputs = doubleOutput      \
    }

static void sink_execute(const void *consumed, void *produced)
{
    UNUSED(consumed);
    UNUSED(produced);
}

#define SINK                       \
    {                              \
        .execution = sink_execute, \
        .numInputs = 1,            \
        .inputs = doubleInput,     \
        .numOutputs = 0,           \
        .outputs = NULL            \
    }

static CsdfActor ACTORS[3] = {THREE_CONSTANT, DOUBLE_GAIN, SINK};

#define CONSTANT_ID 0
#define GAIN_ID 1
#define SINK_ID 2

#define CONSTANT_SOURCE                       \
    {                                         \
        .actorId = CONSTANT_ID, .outputId = 0 \
    }

#define GAIN_DESTINATION                 \
    {                                    \
        .actorId = GAIN_ID, .inputId = 0 \
    }

#define GAIN_SOURCE                       \
    {                                     \
        .actorId = GAIN_ID, .outputId = 0 \
    }

#define SINK_DESTINATION                 \
    {                                    \
        .actorId = SINK_ID, .inputId = 0 \
    }

static CsdfConnection connections[] = {
    {.source = CONSTANT_SOURCE, .destination = GAIN_DESTINATION, .tokenSize = sizeof(double), .numTokens = 0, .initialTokens = NULL},
    {.source = GAIN_SOURCE, .destination = SINK_DESTINATION, .tokenSize = sizeof(double), .numTokens = 0, .initialTokens = NULL}};

const CsdfGraph SIMPLE_GRAPH = {
    .actors = ACTORS,
    .numActors = 3,
    .connections = connections,
    .numConnections = 2};
