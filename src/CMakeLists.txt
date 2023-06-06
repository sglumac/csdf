add_library(csdf STATIC)

target_sources(csdf PRIVATE csdf/repetition.c csdf/execution/sequential.c csdf/execution/parallel.c csdf/execution/actor.c csdf/execution/graph.c csdf/execution/buffer/stdlockfree.c csdf/record.c)
target_include_directories(csdf PUBLIC .)

if(BUILD_CSDF_PTHREAD_SUPPORT)
  add_subdirectory(csdf/execution/parallel/pthread)
  target_link_libraries(csdf PUBLIC csdf_pthread)
endif()
