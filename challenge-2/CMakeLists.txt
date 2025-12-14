add_library(is-prime-lib)

target_sources(
    is-prime-lib
    PRIVATE
    lib/lib.c
    lib/json.h
    lib/lib.h
)

add_executable(is-prime-server)


target_sources(is-prime-server
PRIVATE
main.c
lib/lib.h
)

target_link_libraries(
    is-prime-server
    PRIVATE
    is-prime-lib
    m
)