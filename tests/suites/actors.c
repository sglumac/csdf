/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include <suites/actors.h>

#include <samples/simple.h>

void test_const(YacuTestRun *testRun)
{
    const CsdfActor *threeConst = &SIMPLE_GRAPH.actors[0];

    double *consumed = NULL, produced;

    threeConst->execution(consumed, &produced);

    YACU_ASSERT_EQ_UINT(testRun, threeConst->numInputs, 0);
    YACU_ASSERT_EQ_UINT(testRun, threeConst->numOutputs, 1);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, produced, 3, 1e-6);
}

void test_gain(YacuTestRun *testRun)
{
    const CsdfActor *doubleGain = &SIMPLE_GRAPH.actors[1];

    double consumed = 5;
    double produced;

    doubleGain->execution(&consumed, &produced);

    YACU_ASSERT_EQ_UINT(testRun, doubleGain->numInputs, 1);
    YACU_ASSERT_EQ_UINT(testRun, doubleGain->numOutputs, 1);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, produced, 10, 1e-6);
}

void test_sink(YacuTestRun *testRun)
{
    const CsdfActor *doubleGain = &SIMPLE_GRAPH.actors[2];

    double consumed = 5;
    double produced;

    doubleGain->execution(&consumed, &produced);

    YACU_ASSERT_EQ_UINT(testRun, doubleGain->numInputs, 1);
    YACU_ASSERT_EQ_UINT(testRun, doubleGain->numOutputs, 0);
}

YacuTest actorsTests[] = {
    {"ConstTest", &test_const},
    {"GainTest", &test_gain},
    {"SinkTest", &test_sink},
    END_OF_TESTS};
