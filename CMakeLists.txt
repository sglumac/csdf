cmake_minimum_required(VERSION 3.16)
project(csdf)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_subdirectory(src)
add_subdirectory(tests)
