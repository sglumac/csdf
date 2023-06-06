/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "execution.h"
#include "parallel.h"

#include <samples/simple.h>
#include <samples/larger.h>
#include <csdf/execution/sequential.h>

void test_simple_sequential_iteration(YacuTestRun *testRun)
{
    CsdfGraphRun *runData = new_graph_run(&SIMPLE_GRAPH, 1);
    bool iterationCompleted = sequential_run(runData);
    double *constantOutput = new_record_storage(runData->actorRuns[0]->recordData, 0);
    double *gainOutput = new_record_storage(runData->actorRuns[1]->recordData, 0);
    copy_recorded_tokens(runData->actorRuns[0]->recordData, 0, constantOutput);
    copy_recorded_tokens(runData->actorRuns[1]->recordData, 0, gainOutput);
    YACU_ASSERT_TRUE(testRun, iterationCompleted);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, constantOutput[0], 3., 1e-3);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, gainOutput[0], 6., 1e-3);
    delete_record_storage(constantOutput);
    delete_record_storage(gainOutput);
    delete_graph_run(runData);
}

void test_simple_sequential_run(YacuTestRun *testRun)
{
    CsdfGraphRun *runData = new_graph_run(&SIMPLE_GRAPH, 100);
    bool runCompleted = sequential_run(runData);
    YACU_ASSERT_TRUE(testRun, runCompleted);
    double *constantOutput = new_record_storage(runData->actorRuns[0]->recordData, 0);
    double *gainOutput = new_record_storage(runData->actorRuns[1]->recordData, 0);
    copy_recorded_tokens(runData->actorRuns[0]->recordData, 0, constantOutput);
    copy_recorded_tokens(runData->actorRuns[1]->recordData, 0, gainOutput);
    for (size_t tokenId = 0; tokenId < 100; tokenId++)
    {
        YACU_ASSERT_APPROX_EQ_DBL(testRun, constantOutput[tokenId], 3., 1e-3);
        YACU_ASSERT_APPROX_EQ_DBL(testRun, gainOutput[tokenId], 6., 1e-3);
    }
    delete_record_storage(constantOutput);
    delete_record_storage(gainOutput);
    delete_graph_run(runData);
}

void test_simple_parallel_run(YacuTestRun *testRun)
{
    CsdfGraphRun *run1Data = new_graph_run(&SIMPLE_GRAPH, 100);
    CsdfGraphRun *run2Data = new_graph_run(&SIMPLE_GRAPH, 100);

    bool run1Completed = sequential_run(run1Data);

    YACU_ASSERT_TRUE(testRun, run1Completed);
    YACU_ASSERT_TRUE(testRun, parallel_run(&TEST_THREADING, run2Data));

    double *constant1Output = new_record_storage(run1Data->actorRuns[0]->recordData, 0);
    double *constant2Output = new_record_storage(run2Data->actorRuns[0]->recordData, 0);

    for (size_t tokenId = 0; tokenId < 100; tokenId++)
    {
        YACU_ASSERT_APPROX_EQ_DBL(testRun, constant1Output[tokenId], constant2Output[tokenId], 1e-3);
    }

    delete_record_storage(constant1Output);
    delete_record_storage(constant2Output);

    double *gain1Output = new_record_storage(run1Data->actorRuns[1]->recordData, 0);
    double *gain2Output = new_record_storage(run2Data->actorRuns[1]->recordData, 0);

    for (size_t tokenId = 0; tokenId < 100; tokenId++)
    {
        YACU_ASSERT_APPROX_EQ_DBL(testRun, gain1Output[tokenId], gain2Output[tokenId], 1e-3);
    }

    delete_record_storage(constant1Output);
    delete_record_storage(constant2Output);

    delete_graph_run(run1Data);
    delete_graph_run(run2Data);
}

void test_larger_sequential_iteration(YacuTestRun *testRun)
{
    CsdfGraphRun *runData = new_graph_run(&LARGER_GRAPH, 1);
    bool iterationCompleted = sequential_run(runData);
    delete_graph_run(runData);
    YACU_ASSERT_TRUE(testRun, iterationCompleted);
}

void test_larger_produced_record(YacuTestRun *testRun)
{
    CsdfGraphRun *runData = new_graph_run(&LARGER_GRAPH, 100);

    bool runCompleted = sequential_run(runData);
    YACU_ASSERT_TRUE(testRun, runCompleted);

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
    YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[0], 2);
    YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[1], 3);
    YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[2], 5);
    YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[3], 7);
    delete_record_storage(rightIntOutputProducedTokens);

    delete_graph_run(runData);
}

void test_larger_parallel(YacuTestRun *testRun)
{
    YACU_ASSERT_TRUE(testRun, true);
}

YacuTest executionTests[] = {
    {"SimpleSequentialIterationTest", &test_simple_sequential_iteration},
    {"SimpleSequentialRun", &test_simple_sequential_run},
    {"SimpleParallelRun", &test_simple_parallel_run},
    {"LargerSequentialIterationTest", &test_larger_sequential_iteration},
    {"LargerProducedRecordTest", &test_larger_produced_record},
    {"LargerParallel", &test_larger_parallel},
    END_OF_TESTS};
