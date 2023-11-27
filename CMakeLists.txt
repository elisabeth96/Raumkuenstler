cmake_minimum_required(VERSION 3.26)
project(implicit_meshing)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        polyscope
        GIT_REPOSITORY https://github.com/nmwsharp/polyscope.git)

FetchContent_MakeAvailable(polyscope)

# add include dir for mac
if (APPLE)
    set(LLVM_DIR /opt/homebrew/opt/llvm/lib/cmake/llvm)
endif ()
find_package(LLVM REQUIRED CONFIG)
find_package(TBB REQUIRED)

add_executable(implicit_meshing
        main.cpp
        third_party/imnodes.cpp
        implicit_meshing.cpp
        implicit_meshing.h
        node.cpp
        node.h
        editor.cpp
        editor.h
        compiler.cpp
        compiler.h)

message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")

target_include_directories(implicit_meshing PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(implicit_meshing PRIVATE
        polyscope
        LLVMCore
        LLVMSupport
        LLVMIRReader
        LLVMExecutionEngine
        LLVMMCJIT
        LLVMX86AsmParser
        LLVMX86CodeGen
        LLVMARMCodeGen
        LLVMARMAsmParser
        LLVMAArch64CodeGen
        LLVMAArch64AsmParser
        TBB::tbb
)

target_compile_options(implicit_meshing PRIVATE -march=native)