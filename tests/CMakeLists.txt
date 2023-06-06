add_executable(tests tests.c samples/simple.c samples/larger.c suites/actors.c suites/graph.c suites/execution.c suites/parallel.c)

include(FetchContent)

FetchContent_Declare(
  yacutest
  GIT_REPOSITORY https://github.com/sglumac/yacutest.git
  GIT_TAG main
)
FetchContent_MakeAvailable(yacutest)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(tests csdf yacu Threads::Threads)
target_include_directories(tests PRIVATE .)
