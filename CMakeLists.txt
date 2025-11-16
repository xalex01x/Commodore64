cmake_minimum_required(VERSION 3.16)
project(Commodore64)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# SDL2 con supporto CMake corretto
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.28.5
    GIT_SHALLOW FALSE 
)
FetchContent_MakeAvailable(SDL2)

add_executable(Commodore64 main.cpp)

target_link_libraries(Commodore64
    PRIVATE
        SDL2::SDL2main
        SDL2::SDL2
)

add_custom_command(TARGET Commodore64 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:SDL2::SDL2>"
    "$<TARGET_FILE_DIR:Commodore64>"
)

