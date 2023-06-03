/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "repetition.h"

#include <stdlib.h>

typedef struct Rational
{
    unsigned int num;
    unsigned int den;
} Rational;

static unsigned int gcd(unsigned int a, unsigned int b)
{
    int temp;
    while (b != 0)
    {
        temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

static unsigned int lcm(unsigned int a, unsigned int b)
{
    unsigned int result = (a * b) / gcd(a, b);
    return result;
}

static void set_rational_value(Rational *rational, unsigned int num, unsigned int den)
{
    rational->num = num;
    rational->den = den;
}

static void reduce_rational_value(Rational *rational)
{
    unsigned int div = gcd(rational->num, rational->den);
    rational->num /= div;
    rational->den /= div;
}

static void set_reduce_rational_value(Rational *rational, unsigned int num, unsigned int den)
{
    set_rational_value(rational, num, den);
    reduce_rational_value(rational);
}

static bool is_rational_zero(const Rational *rational)
{
    return rational->num == 0;
}

static unsigned int rational_eq(const Rational *rational, unsigned int num, unsigned int den)
{
    return rational->num * den == rational->den * num;
}

static void get_rate_ratio(
    const CsdfGraph *graph, const CsdfConnection *connection,
    unsigned int *production,
    unsigned int *consumption)
{
    const CsdfInputId *dstId = &connection->destination;
    const CsdfInput *dstPort = &graph->actors[dstId->actorId].inputs[dstId->inputId];
    const CsdfOutputId *srcId = &connection->source;
    const CsdfOutput *srcPort = &graph->actors[srcId->actorId].outputs[srcId->outputId];
    *production = srcPort->production;
    *consumption = dstPort->consumption;
}

static bool fill_candidate_vector(
    const CsdfGraph *graph, size_t pivotId,
    Rational *candidateVector)
{
    for (size_t connectionId = 0; connectionId < graph->numConnections; connectionId++)
    {
        const CsdfConnection *connection = &graph->connections[connectionId];
        Rational *dstVal = &candidateVector[connection->destination.actorId];
        Rational *srcVal = &candidateVector[connection->source.actorId];
        unsigned int production, consumption;
        get_rate_ratio(graph, connection, &production, &consumption);
        if (connection->source.actorId == pivotId)
        {
            unsigned int dstNum = srcVal->num * production;
            unsigned int dstDen = srcVal->den * consumption;

            if (is_rational_zero(dstVal))
            {
                set_reduce_rational_value(dstVal, dstNum, dstDen);
                if (!fill_candidate_vector(graph, connection->destination.actorId, candidateVector))
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
        if (connection->destination.actorId == pivotId)
        {
            unsigned int srcNum = dstVal->num * consumption;
            unsigned int srcDen = dstVal->den * production;
            if (is_rational_zero(srcVal))
            {
                set_reduce_rational_value(srcVal, srcNum, srcDen);
                if (!fill_candidate_vector(graph, connection->source.actorId, candidateVector))
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