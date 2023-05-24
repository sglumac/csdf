/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "repetition.h"
#include "utility.h"

#include <stdlib.h>

static void get_rate_ratio(
    const CsdfGraph *graph, const CsdfBuffer *buffer,
    unsigned int *production,
    unsigned int *consumption)
{
    const CsdfInputId *dstId = &buffer->destination;
    const CsdfInput *dstPort = &graph->actors[dstId->actorId].inputs[dstId->inputId];
    const CsdfOutputId *srcId = &buffer->source;
    const CsdfOutput *srcPort = &graph->actors[srcId->actorId].outputs[srcId->outputId];
    *production = srcPort->production;
    *consumption = dstPort->consumption;
}

static bool fill_candidate_vector(
    const CsdfGraph *graph, size_t pivotId,
    Rational *candidateVector)
{
    for (size_t bufferId = 0; bufferId < graph->numBuffers; bufferId++)
    {
        const CsdfBuffer *buffer = &graph->buffers[bufferId];
        Rational *dstVal = &candidateVector[buffer->destination.actorId];
        Rational *srcVal = &candidateVector[buffer->source.actorId];
        unsigned int production, consumption;
        get_rate_ratio(graph, buffer, &production, &consumption);
        if (buffer->source.actorId == pivotId)
        {
            unsigned int dstNum = srcVal->num * production;
            unsigned int dstDen = srcVal->den * consumption;

            if (is_rational_zero(dstVal))
            {
                set_reduce_rational_value(dstVal, dstNum, dstDen);
                if (!fill_candidate_vector(graph, buffer->destination.actorId, candidateVector))
                {
                    return false;
                }
            }
            else
            {
                if (!rational_eq(dstVal, dstNum, dstDen))
                {
                    return false;
                }
            }
        }
        if (buffer->destination.actorId == pivotId)
        {
            unsigned int srcNum = dstVal->num * consumption;
            unsigned int srcDen = dstVal->den * production;
            if (is_rational_zero(srcVal))
            {
                set_reduce_rational_value(srcVal, srcNum, srcDen);
                if (!fill_candidate_vector(graph, buffer->source.actorId, candidateVector))
                {
                    return false;
                }
            }
            else
            {
                if (!rational_eq(srcVal, srcNum, srcDen))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

static bool any_zero(const Rational *candidateVector, size_t numActors)
{
    for (size_t actorId = 0; actorId < numActors; actorId++)
    {
        if (is_rational_zero(&candidateVector[actorId]))
        {
            return true;
        }
    }
    return false;
}

static void fill_repetition_vector(const Rational *candidateVector, size_t numActors, unsigned int *repetitionVector)
{
    unsigned int multiple = 1;
    for (size_t actorId = 0; actorId < numActors; actorId++)
    {
        multiple = lcm(multiple, candidateVector[actorId].den);
    }
    for (size_t actorId = 0; actorId < numActors; actorId++)
    {
        repetitionVector[actorId] = multiple * candidateVector[actorId].num / candidateVector[actorId].den;
    }
}

bool csdf_repetition_vector(const CsdfGraph *graph, unsigned int *repetitionVector)
{
    Rational *candidateVector = calloc(graph->numActors, sizeof(Rational));

    size_t pivotId = 0;
    set_rational_value(&candidateVector[pivotId], 1, 1);
    if (!fill_candidate_vector(graph, pivotId, candidateVector) || any_zero(candidateVector, graph->numActors))
    {
        free(candidateVector);
        return false;
    }
    else
    {
        fill_repetition_vector(candidateVector, graph->numActors, repetitionVector);
        free(candidateVector);
        return true;
    }
}