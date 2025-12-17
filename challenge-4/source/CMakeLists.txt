
# Library Config

add_library(challenge-4-lib)


target_sources(challenge-4-lib
PRIVATE
server.c
chat.c
)

target_include_directories(
    challenge-4-lib
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)


# Final Binary

add_executable(challenge-4)

target_sources(challenge-4
PRIVATE
main.c
)

target_include_directories(
    challenge-4
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(challenge-4
PRIVATE
challenge-4-lib
)

