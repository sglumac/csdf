/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include <suites/sequential.h>
#include <samples/simple.h>
#include <samples/larger.h>
#include <csdf/sequential.h>

#define UNUSED(x) (void)(x)

typedef struct SimpleResultsState
{
    bool fired[3];
    double produced[3];
    double consumed[3];
} SimpleResultsState;

void on_simple_token_produced(const CsdfGraph *graph, void *recordState, size_t actorId, void *produced)
{
    UNUSED(graph);
    SimpleResultsState *sampleResultsState = recordState;
    const CsdfActor *actor = &graph->actors[actorId];
    sampleResultsState->fired[actorId] = true;
    if (actor->numOutputs > 0)
    {
        double *token = produced;
        sampleResultsState->produced[actorId] = *token;
    }
}

void on_simple_token_consumed(const CsdfGraph *graph, void *recordState, size_t actorId, void *consumed)
{
    UNUSED(graph);
    SimpleResultsState *sampleResultsState = recordState;
    const CsdfActor *actor = &graph->actors[actorId];
    sampleResultsState->fired[actorId] = true;
    if (actor->numInputs > 0)
    {
        double *token = consumed;
        sampleResultsState->consumed[actorId] = *token;
    }
}

void test_simple_fire(YacuTestRun *testRun)
{
    SimpleResultsState recordState = {.fired = {false}, .consumed = {0.}, .produced = {0.}};
    CsdfRecordOptions options = {
        .on_token_consumed = on_simple_token_consumed,
        .on_token_produced = on_simple_token_produced,
        .recordState = &recordState};
    CsdfGraphState *state = new_sequential_state(&SIMPLE_GRAPH);
    YACU_ASSERT_TRUE(testRun, can_fire(state, 0));
    fire(&options, state, 0);
    delete_sequential_state(state);

    YACU_ASSERT_TRUE(testRun, recordState.fired[0]);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, recordState.produced[0], 3., 1e-3);
}

void test_simple_sequential_iteration(YacuTestRun *testRun)
{
    SimpleResultsState recordState = {.fired = {false}, .consumed = {0.}, .produced = {0.}};
    CsdfRecordOptions options = {
        .on_token_consumed = on_simple_token_consumed,
        .on_token_produced = on_simple_token_produced,
        .recordState = &recordState};
    CsdfGraphState *state = new_sequential_state(&SIMPLE_GRAPH);
    bool iterationCompleted = sequential_iteration(&options, state);
    delete_sequential_state(state);
    YACU_ASSERT_TRUE(testRun, iterationCompleted);
    YACU_ASSERT_TRUE(testRun, recordState.fired[0]);
    YACU_ASSERT_TRUE(testRun, recordState.fired[1]);
    YACU_ASSERT_TRUE(testRun, recordState.fired[2]);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, recordState.produced[0], 3., 1e-3);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, recordState.produced[1], 6., 1e-3);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, recordState.consumed[1], 3., 1e-3);
    YACU_ASSERT_APPROX_EQ_DBL(testRun, recordState.consumed[2], 6., 1e-3);
}

typedef struct LargerResultsState
{
    unsigned int fired[2];
} LargerResultsState;

void on_larger_token_produced(const CsdfGraph *graph, void *recordState, size_t actorId, void *produced)
{
    UNUSED(graph);
    UNUSED(recordState);
    UNUSED(actorId);
    UNUSED(produced);
}

void on_larger_token_consumed(const CsdfGraph *graph, void *recordState, size_t actorId, void *consumed)
{
    UNUSED(graph);
    UNUSED(consumed);
    LargerResultsState *largerResultsState = recordState;
    largerResultsState->fired[actorId]++;
}

void test_larger_sequential_iteration(YacuTestRun *testRun)
{
    LargerResultsState recordState = {.fired = {0}};
    CsdfRecordOptions options = {
        .on_token_consumed = on_larger_token_consumed,
        .on_token_produced = on_larger_token_produced,
        .recordState = &recordState};
    CsdfGraphState *state = new_sequential_state(&LARGER_GRAPH);
    bool iterationCompleted = sequential_iteration(&options, state);
    delete_sequential_state(state);
    YACU_ASSERT_TRUE(testRun, iterationCompleted);
    YACU_ASSERT_EQ_UINT(testRun, recordState.fired[0], 2);
    YACU_ASSERT_EQ_UINT(testRun, recordState.fired[1], 1);
}

void test_larger_produced_record(YacuTestRun *testRun)
{
    CsdfRecordOptions *recordOptions = new_record_produced_options(&LARGER_GRAPH, 100);
    CsdfGraphState *state = new_sequential_state(&LARGER_GRAPH);
    bool iterationCompleted = sequential_iteration(recordOptions, state);
    delete_sequential_state(state);

    int *leftIntOutputProducedTokens = new_record_storage(recordOptions, 0, 1);
    copy_recorded_produced_tokens(recordOptions, 0, 1, leftIntOutputProducedTokens);
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

    double *leftDoubleOutputProducedTokens = new_record_storage(recordOptions, 0, 0);
    copy_recorded_produced_tokens(recordOptions, 0, 0, leftDoubleOutputProducedTokens);
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

    char *rightCharOutputProducedTokens = new_record_storage(recordOptions, 1, 0);
    copy_recorded_produced_tokens(recordOptions, 1, 0, rightCharOutputProducedTokens);
    for (size_t i = 0; i < 3; i++)
    {
        YACU_ASSERT_EQ_CHAR(testRun, rightCharOutputProducedTokens[i], (char)leftDoubleOutputProducedTokens[i]);
        YACU_ASSERT_EQ_CHAR(testRun, rightCharOutputProducedTokens[i + 3], (char)leftDoubleOutputProducedTokens[i + 5]);
    }
    delete_record_storage(leftDoubleOutputProducedTokens);
    delete_record_storage(rightCharOutputProducedTokens);

    int *rightIntOutputProducedTokens = new_record_storage(recordOptions, 1, 1);
    copy_recorded_produced_tokens(recordOptions, 1, 1, rightIntOutputProducedTokens);
    YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[0], 2);
    YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[1], 3);
    YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[2], 5);
    YACU_ASSERT_EQ_INT(testRun, rightIntOutputProducedTokens[3], 7);
    delete_record_storage(rightIntOutputProducedTokens);

    delete_record_produced_options(&LARGER_GRAPH, recordOptions);
    YACU_ASSERT_TRUE(testRun, iterationCompleted);
}

YacuTest sequentialTests[] = {
    {"SimpleFireTest", &test_simple_fire},
    {"SimpleSequentialIterationTest", &test_simple_sequential_iteration},
    {"LargerSequentialIterationTest", &test_larger_sequential_iteration},
    {"LargerProducedRecordTest", &test_larger_produced_record},
    END_OF_TESTS};
