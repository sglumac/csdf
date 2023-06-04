add_library(csdf STATIC)

target_sources(csdf PRIVATE csdf/repetition.c csdf/execution/sequential.c csdf/execution/actor.c csdf/execution/buffer.c csdf/record.c)
target_include_directories(csdf PUBLIC .)
