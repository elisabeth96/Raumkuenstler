cmake_minimum_required(VERSION 3.26)
project(implicit_meshing)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        polyscope
        GIT_REPOSITORY https://github.com/nmwsharp/polyscope.git)

FetchContent_MakeAvailable(polyscope)

add_executable(implicit_meshing main.cpp)
target_link_libraries(implicit_meshing PRIVATE polyscope)

target_compile_options(implicit_meshing PRIVATE -march=native)