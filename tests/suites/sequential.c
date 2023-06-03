/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include <suites/sequential.h>
#include <samples/simple.h>
#include <samples/larger.h>
#include <csdf/execution/sequential.h>

void test_simple_sequential_iteration(YacuTestRun *testRun)
{
    CsdfSequentialRun *runData = new_sequential_run(&SIMPLE_GRAPH, 1);
    bool iterationCompleted = sequential_iteration(runData);
    double *constantOutput = new_record_storage(runData->actorRuns[0]->recordData, 0);
    double *gainOutput = new_record_storage(runData->actorRuns[1]->recordData, 0);
    copy_recorded_tokens(runData->actorRuns[0]->recordData, 0, constantOutput);
    copy_recorded_tokens(runData->actorRuns[1]->recordData, 0, gainOutput);
    YACU_ASSERT_TRUE(testRun, iterationCompleted);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, constantOutput[0], 3., 1e-3);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, gainOutput[0], 6., 1e-3);
    delete_record_storage(constantOutput);
    delete_record_storage(gainOutput);
    delete_sequential_run(runData);
}

void test_larger_sequential_iteration(YacuTestRun *testRun)
{
    CsdfSequentialRun *runData = new_sequential_run(&LARGER_GRAPH, 1);
    bool iterationCompleted = sequential_iteration(runData);
    delete_sequential_run(runData);
    YACU_ASSERT_TRUE(testRun, iterationCompleted);
}

void test_larger_produced_record(YacuTestRun *testRun)
{
    CsdfSequentialRun *runData = new_sequential_run(&LARGER_GRAPH, 100);

    YACU_ASSERT_TRUE(testRun, sequential_iteration(runData));

    int *leftIntOutputProducedTokens = new_record_storage(runData->actorRuns[0]->recordData, 1);
    copy_recorded_tokens(runData->actorRuns[0]->recordData, 1, leftIntOutputProducedTokens);
    YACU_ASSERT_EQ_INT(testRun, leftIntOutputProducedTokens[0], 2);
    for (size_t i = 1; i < 7; i++)
    {
        YACU_ASSERT_EQ_INT(testRun, leftIntOutputProducedTokens[i], 3);
    }
    YACU_ASSERT_EQ_INT(testRun, leftIntOutputProducedTokens[7], 5);
    for (size_t i = 8; i < 14; i++)
    {
        YACU_ASSERT_EQ_INT(testRun, leftIntOutputProducedTokens[i], 7);
    }
    delete_record_storage(leftIntOutputProducedTokens);

    double *leftDoubleOutputProducedTokens = new_record_storage(runData->actorRuns[0]->recordData, 0);
    copy_recorded_tokens(runData->actorRuns[0]->recordData, 0, leftDoubleOutputProducedTokens);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, leftDoubleOutputProducedTokens[0], (double)'a', 1e-3);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, leftDoubleOutputProducedTokens[1], (double)'b', 1e-3);
    for (size_t i = 2; i < 5; i++)
    {
        YACU_ASSERT_APPROX_EQ_DBL(testRun, leftDoubleOutputProducedTokens[i], (double)'c', 1e-3);
    }
    YACU_ASSERT_APPROX_EQ_DBL(testRun, leftDoubleOutputProducedTokens[5], (double)'d', 1e-3);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, leftDoubleOutputProducedTokens[6], (double)'e', 1e-3);
    for (size_t i = 7; i < 10; i++)
    {
        YACU_ASSERT_APPROX_EQ_DBL(testRun, leftDoubleOutputProducedTokens[i], (double)'f', 1e-3);
    }

    char *rightCharOutputProducedTokens = new_record_storage(runData->actorRuns[1]->recordData, 0);
    copy_recorded_tokens(runData->actorRuns[1]->recordData, 0, rightCharOutputProducedTokens);
    for (size_t i = 0; i < 3; i++)
    {
        YACU_ASSERT_EQ_CHAR(testRun, rightCharOutputProducedTokens[i], (char)leftDoubleOutputProducedTokens[i]);
        YACU_ASSERT_EQ_CHAR(testRun, rightCharOutputProducedTokens[i + 3], (char)leftDoubleOutputProducedTokens[i + 5]);
    }
    delete_record_storage(leftDoubleOutputProducedTokens);
    delete_record_storage(rightCharOutputProducedTokens);

    int *rightIntOutputProducedTokens = new_record_storage(runData->actorRuns[1]->recordData, 1);
    copy_recorded_tokens(runData->actorRuns[1]->recordData, 1, rightIntOutputProducedTokens);
    // YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[0], 2);
    // YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[1], 3);
    // YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[2], 5);
    // YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[3], 7);
    delete_record_storage(rightIntOutputProducedTokens);

    delete_sequential_run(runData);
}

YacuTest sequentialTests[] = {
    {"SimpleSequentialIterationTest", &test_simple_sequential_iteration},
    {"LargerSequentialIterationTest", &test_larger_sequential_iteration},
    {"LargerProducedRecordTest", &test_larger_produced_record},
    END_OF_TESTS};
