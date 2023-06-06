add_executable(tests tests.c samples/simple.c samples/larger.c suites/actors.c suites/graph.c suites/execution.c)

include(FetchContent)

FetchContent_Declare(
  yacutest
  GIT_REPOSITORY https://github.com/sglumac/yacutest.git
  GIT_TAG main
)
FetchContent_MakeAvailable(yacutest)

FetchContent_Declare(
  threading4csdf
  GIT_REPOSITORY https://github.com/sglumac/threading4csdf.git
  GIT_TAG main
)
FetchContent_MakeAvailable(threading4csdf)

target_link_libraries(tests csdf yacu pthread4csdf)
target_include_directories(tests PRIVATE .)
