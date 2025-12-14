cmake_minimum_required(VERSION 3.28.3)

project(echo-server
    VERSION 0.0.1
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -pedantic)

add_executable(writer-client)

target_sources(writer-client
    PRIVATE
    client.c)

add_executable(writer-server)

target_sources(writer-server
    PRIVATE
    server.c)