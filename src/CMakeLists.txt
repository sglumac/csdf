add_library(csdf STATIC)

target_sources(csdf PRIVATE csdf/repetition.c csdf/utility.c csdf/execution/sequential.c csdf/record.c)
target_include_directories(csdf PUBLIC .)
