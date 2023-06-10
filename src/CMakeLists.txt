add_library(csdf STATIC)

target_sources(csdf PRIVATE csdf/repetition.c csdf/execution/sequential.c csdf/execution/parallel.c csdf/execution/actorrun.c csdf/execution/graphrun.c csdf/execution/buffer/stdlockfree.c csdf/record.c)
target_include_directories(csdf PUBLIC .)

include(FetchContent)

FetchContent_Declare(
  threading4csdf
  GIT_REPOSITORY https://github.com/sglumac/threading4csdf.git
  GIT_TAG main
)
FetchContent_MakeAvailable(threading4csdf)

target_link_libraries(csdf PRIVATE threading4csdf)
