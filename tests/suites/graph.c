/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include <suites/graph.h>

#include <samples/simple.h>
#include <samples/larger.h>

#include <csdf/repetition.h>

void test_simple_repetition_vector(YacuTestRun *testRun)
{
    unsigned int r[3] = {0};

    YACU_ASSERT_TRUE(testRun, csdf_repetition_vector(&SIMPLE_GRAPH, r));

    YACU_ASSERT_EQ_UINT(testRun, r[0], 1);
    YACU_ASSERT_EQ_UINT(testRun, r[1], 1);
    YACU_ASSERT_EQ_UINT(testRun, r[2], 1);
}

void test_larger_repetition_vector(YacuTestRun *testRun)
{
    unsigned int r[2] = {0};

    YACU_ASSERT_TRUE(testRun, csdf_repetition_vector(&LARGER_GRAPH, r));

    YACU_ASSERT_EQ_UINT(testRun, r[0], 2);
    YACU_ASSERT_EQ_UINT(testRun, r[1], 1);
}

YacuTest graphTests[] = {
    {"SimpleRepetitionVectorTest", &test_simple_repetition_vector},
    {"LargerRepetitionVectorTest", &test_larger_repetition_vector},
    END_OF_TESTS};
