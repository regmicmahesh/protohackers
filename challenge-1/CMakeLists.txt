
add_executable(writer-client)

target_sources(writer-client
    PRIVATE
    client.c)

add_executable(writer-server)

target_sources(writer-server
    PRIVATE
    server.c)