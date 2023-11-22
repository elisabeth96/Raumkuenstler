cmake_minimum_required(VERSION 3.26)
project(implicit_meshing)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        polyscope
        GIT_REPOSITORY https://github.com/nmwsharp/polyscope.git)

FetchContent_MakeAvailable(polyscope)

find_package(LLVM REQUIRED CONFIG)

add_executable(implicit_meshing main.cpp imnodes.cpp
        implicit_meshing.cpp
        implicit_meshing.h
        node.cpp
        node.h
        editor.cpp
        editor.h
        compiler.cpp
        compiler.h)

#add_executable(test_compiler test.cpp compiler.cpp compiler.h)

target_link_libraries(implicit_meshing PRIVATE polyscope LLVMCore LLVMSupport LLVMIRReader LLVMExecutionEngine LLVMMCJIT LLVMX86AsmParser LLVMX86CodeGen)
#target_link_libraries(test_compiler PRIVATE LLVMCore LLVMSupport LLVMIRReader LLVMExecutionEngine LLVMMCJIT LLVMX86AsmParser LLVMX86CodeGen)

target_compile_options(implicit_meshing PRIVATE -march=native)