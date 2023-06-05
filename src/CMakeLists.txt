add_library(csdf STATIC)

target_sources(csdf PRIVATE csdf/repetition.c csdf/execution/sequential.c csdf/execution/parallel.c csdf/execution/actor.c csdf/execution/graph.c csdf/execution/buffer/stdlockfree.c csdf/record.c)
target_include_directories(csdf PUBLIC .)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(csdf PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(csdf "${CMAKE_THREAD_LIBS_INIT}")
endif()
