

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