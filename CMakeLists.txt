cmake_minimum_required(VERSION 3.28)

project(protohackers
    VERSION 0.0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -pedantic)

add_subdirectory(challenge-1)
add_subdirectory(challenge-2)