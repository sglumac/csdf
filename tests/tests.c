/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include <yacu.h>

#include <suites/actors.h>
#include <suites/graph.h>
#include <suites/execution.h>

YacuSuite suites[] = {
    {"ActorsSuite", actorsTests},
    {"GraphSuite", graphTests},
    {"ExecutionSuite", executionTests},
    END_OF_SUITES};

int main(int argc, char const *argv[])
{
    YacuOptions options = yacu_process_args(argc, argv);
    return yacu_execute(options, suites);
}
