cmake_minimum_required(VERSION 3.28)

project(
    challenge-4
    VERSION 0.0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options(-Wall -Wextra -pedantic)

enable_testing()

add_subdirectory(source)


