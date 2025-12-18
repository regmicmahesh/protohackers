cmake_minimum_required(VERSION 3.28)

project(tcp-chat
  VERSION 0.0.1
  LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_compile_options(-Wall -Wextra -pedantic)

add_executable(tcp-chat)

target_sources(tcp-chat
PRIVATE
  main.c
)