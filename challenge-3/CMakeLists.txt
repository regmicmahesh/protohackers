add_executable(chall-3-server)

add_library(lib)

target_sources(lib
PRIVATE
lib/lib.c
)


target_sources(chall-3-server
PRIVATE
main.c
lib/lib.h
)

target_link_libraries(
    chall-3-server
    PRIVATE
    lib
)