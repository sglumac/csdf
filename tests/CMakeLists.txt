add_executable(tests tests.c samples/simple.c samples/larger.c suites/actors.c suites/graph.c suites/execution.c)

include(FetchContent)

FetchContent_Declare(
  yacutest
  GIT_REPOSITORY https://github.com/sglumac/yacutest.git
  GIT_TAG main
)
FetchContent_MakeAvailable(yacutest)

FetchContent_Declare(
  pthread4csdf
  GIT_REPOSITORY https://github.com/sglumac/pthread4csdf.git
  GIT_TAG main
)
FetchContent_MakeAvailable(pthread4csdf)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(tests csdf yacu pthread4csdf Threads::Threads)
target_include_directories(tests PRIVATE .)
