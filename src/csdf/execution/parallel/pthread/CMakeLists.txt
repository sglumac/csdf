add_library(csdf_pthread STATIC)

target_sources(csdf_pthread PRIVATE pthread.c)
target_include_directories(csdf_pthread PUBLIC ../../../..)
