/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "larger.h"
#include <stdint.h>

#define UNUSED(x) (void)(x)

CsdfInput leftInputs[] = {CSDF_INPUT(int, 2), CSDF_INPUT(char, 3)};

CsdfOutput leftOutputs[] = {CSDF_OUTPUT(double, 5), CSDF_OUTPUT(int, 7)};

static void left_execute(const void *consumed, void *produced)
{
    const int *intInputTokens = consumed;
    char *charInputTokens = (char *)(((uint8_t *)consumed) + 2 * sizeof(int));

    double *doubleOutputTokens = produced;
    int *intOutputTokens = (int *)(((uint8_t *)produced) + 5 * sizeof(double));

    intOutputTokens[0] = intInputTokens[0];
    for (size_t tokenId = 1; tokenId < 7; tokenId++)
    {
        intOutputTokens[tokenId] = intInputTokens[1];
    }
    doubleOutputTokens[0] = charInputTokens[0];
    doubleOutputTokens[1] = charInputTokens[1];
    for (size_t tokenId = 2; tokenId < 5; tokenId++)
    {
        doubleOutputTokens[tokenId] = charInputTokens[2];
    }
}

#define LEFT                       \
    {                              \
        .execution = left_execute, \
        .numInputs = 2,            \
        .inputs = leftInputs,      \
        .numOutputs = 2,           \
        .outputs = leftOutputs     \
    }

CsdfInput rightInputs[] = {CSDF_INPUT(int, 14), CSDF_INPUT(double, 10)};

CsdfOutput rightOutputs[] = {CSDF_OUTPUT(char, 6), CSDF_OUTPUT(int, 4)};

static void right_execute(const void *consumed, void *produced)
{
    const int *intInputTokens = consumed;
    const double *doubleInputTokens = (double *)(((uint8_t *)consumed) + 14 * sizeof(int));

    char *charOutputTokens = produced;
    int *intOutputTokens = (int *)(((uint8_t *)produced) + 6 * sizeof(char));

    for (size_t tokenId = 0; tokenId < 3; tokenId++)
    {
        charOutputTokens[tokenId] = doubleInputTokens[tokenId];
        charOutputTokens[tokenId + 3] = doubleInputTokens[tokenId + 5];
    }

    intOutputTokens[0] = intInputTokens[0];
    intOutputTokens[1] = intInputTokens[1];
    intOutputTokens[2] = intInputTokens[7];
    intOutputTokens[3] = intInputTokens[8];
}

#define RIGHT                       \
    {                               \
        .execution = right_execute, \
        .numInputs = 2,             \
        .inputs = rightInputs,      \
        .numOutputs = 2,            \
        .outputs = rightOutputs     \
    }

static CsdfActor ACTORS[2] = {LEFT, RIGHT};

#define DOUBLE_LEFT_TO_RIGHT                         \
    {                                                \
        .source = {.actorId = 0, .outputId = 0},     \
        .destination = {.actorId = 1, .inputId = 1}, \
        .tokenSize = sizeof(double),                 \
        .numTokens = 0,                              \
        .initialTokens = NULL                        \
    }

#define INT_LEFT_TO_RIGHT                            \
    {                                                \
        .source = {.actorId = 0, .outputId = 1},     \
        .destination = {.actorId = 1, .inputId = 0}, \
        .tokenSize = sizeof(int),                    \
        .numTokens = 0,                              \
        .initialTokens = NULL                        \
    }

static int intRight2Left[] = {2, 3, 5, 7};

#define INT_RIGHT_TO_LEFT                            \
    {                                                \
        .source = {.actorId = 1, .outputId = 1},     \
        .destination = {.actorId = 0, .inputId = 0}, \
        .tokenSize = sizeof(int),                    \
        .numTokens = 4,                              \
        .initialTokens = intRight2Left               \
    }

static char charRight2Left[] = {'a', 'b', 'c', 'd', 'e', 'f'};

#define CHAR_RIGHT_TO_LEFT                           \
    {                                                \
        .source = {.actorId = 1, .outputId = 0},     \
        .destination = {.actorId = 0, .inputId = 1}, \
        .tokenSize = sizeof(char),                   \
        .numTokens = 6,                              \
        .initialTokens = charRight2Left              \
    }

static CsdfBuffer buffers[] = {
    DOUBLE_LEFT_TO_RIGHT,
    INT_LEFT_TO_RIGHT,
    CHAR_RIGHT_TO_LEFT,
    INT_RIGHT_TO_LEFT};

const CsdfGraph LARGER_GRAPH = {
    .actors = ACTORS,
    .numActors = 2,
    .buffers = buffers,
    .numBuffers = 4};
