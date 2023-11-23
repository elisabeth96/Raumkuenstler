//
// Created by elisabeth on 21.11.23.
//

#include "compiler.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <functional>
#include <map>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>


std::function<double(glm::dvec3)> compile(std::vector<Instruction>& instructions, std::map<int, double>& constants) {
    // Initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::LLVMContext context;
    llvm::IRBuilder<> builder(context);

    // enable fast math
    llvm::FastMathFlags fast_flags;
    fast_flags.setFast();
    builder.setFastMathFlags(fast_flags);

    std::unique_ptr<llvm::Module> module(new llvm::Module("mathModule", context));

    // Function signature
    std::vector<llvm::Type*> args_types(3, llvm::Type::getDoubleTy(context));
    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), args_types, false);
    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "mathFunc", module.get());

    // Entry Block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", function);
    builder.SetInsertPoint(entry);

    // Process Instructions
    // A map to keep track of values (variables and constants) in the function
    std::map<int, llvm::Value*> valueMap;

    // Function arguments
    auto args = function->arg_begin();
    llvm::Value* arg1 = args++;
    llvm::Value* arg2 = args++;
    llvm::Value* arg3 = args++;
    valueMap[0] = arg1;
    valueMap[1] = arg2;
    valueMap[2] = arg3;

    // Load constants into valueMap
    for (const auto& kv : constants) {
        valueMap[kv.first] = llvm::ConstantFP::get(context, llvm::APFloat(kv.second));
    }

    // Process each instruction
    for (const auto& instr : instructions) {
        llvm::Value* lhs = valueMap[instr.input1];
        llvm::Value* rhs = (instr.input2 != -1) ? valueMap[instr.input2] : nullptr;
        llvm::Value* result = nullptr;

        switch (instr.operation) {
            case Operation::Add:
                result = builder.CreateFAdd(lhs, rhs, "addtmp");
                break;
            case Operation::Sub:
                result = builder.CreateFSub(lhs, rhs, "subtmp");
                break;
            case Operation::Mul:
                result = builder.CreateFMul(lhs, rhs, "multmp");
                break;
            case Operation::Sqrt:
                // Assuming sqrt function is available in your LLVM setup
                result = builder.CreateCall(module->getOrInsertFunction("llvm.sqrt.f64", funcType), lhs, "sqrttmp");
                break;
            case Operation::Min:
                // Implement Min operation
                result = builder.CreateSelect(builder.CreateFCmpULT(lhs, rhs), lhs, rhs);
                break;
            case Operation::Max:
                // Implement Max operation
                result = builder.CreateSelect(builder.CreateFCmpULT(lhs, rhs), rhs, lhs);
                break;
            case Operation::Abs:
                // Implement Abs operation
                result = builder.CreateSelect(builder.CreateFCmpULT(lhs, llvm::ConstantFP::get(context, llvm::APFloat(0.0))), builder.CreateFNeg(lhs), lhs);
                break;
            case Operation::Sin:
                // Implement Sin operation
                result = builder.CreateCall(module->getOrInsertFunction("llvm.sin.f64", funcType), lhs, "sintmp");
                break;
            case Operation::Cos:
                // Implement Cos operation
                result = builder.CreateCall(module->getOrInsertFunction("llvm.cos.f64", funcType), lhs, "costmp");
                break;
            default:
                // Handle unknown operation
                assert(false && "Unknown operation");
                break;
        }

        // Store the result in the value map
        valueMap[instr.output] = result;
    }

    // The final instruction should put its result in `valueMap[output]` for the return value
    builder.CreateRet(valueMap[instructions.back().output]);

    // Verify the function
    llvm::verifyFunction(*function);

    auto start_compile = std::chrono::high_resolution_clock::now();

    // Compile the function
    std::string errMsg;
    llvm::ExecutionEngine* engine = llvm::EngineBuilder(std::move(module)).setOptLevel(llvm::CodeGenOpt::Aggressive).setErrorStr(&errMsg).create();
    if (!engine) {
        // Handle error
        throw std::runtime_error(errMsg);
    }

    engine->finalizeObject();
    void* funcPtr = engine->getPointerToFunction(function);

    auto end_compile = std::chrono::high_resolution_clock::now();
    printf("Finalizing: %f ms\n", std::chrono::duration<double, std::milli>(end_compile - start_compile).count());

    typedef double (*FuncType)(double, double, double);
    auto func = reinterpret_cast<FuncType>(funcPtr);
    return [func](glm::dvec3 p) -> double {
        return func(p.x, p.y, p.z);
    };
}

int make_constant(std::map<int, double>& constants, int& current_register, double value) {
    int id = current_register++;
    constants[id] = value;
    return id;
}

int generate_length(std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v) {
    Instruction i1 = {v.x, v.x, current_register++, Operation::Mul};
    Instruction i2 = {v.y, v.y, current_register++, Operation::Mul};
    Instruction i3 = {v.z, v.z, current_register++, Operation::Mul};
    Instruction i4 = {i1.output, i2.output, current_register++, Operation::Add};
    Instruction i5 = {i4.output, i3.output, current_register++, Operation::Add};
    Instruction i6 = {i5.output, -1, current_register++, Operation::Sqrt};
    instructions.insert(instructions.end(), {i1, i2, i3, i4, i5, i6});
    return i6.output;
}

int generate_length(std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v) {
    Instruction i1 = {v.x, v.x, current_register++, Operation::Mul};
    Instruction i2 = {v.y, v.y, current_register++, Operation::Mul};
    Instruction i3 = {i1.output, i2.output, current_register++, Operation::Add};
    Instruction i4 = {i3.output, -1, current_register++, Operation::Sqrt};
    instructions.insert(instructions.end(), {i1, i2, i3, i4});
    return i4.output;
}

glm::ivec3 generate_sub (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1, glm::ivec3 v2) {
    Instruction i1 = {v1.x, v2.x, current_register++, Operation::Sub};
    Instruction i2 = {v1.y, v2.y, current_register++, Operation::Sub};
    Instruction i3 = {v1.z, v2.z, current_register++, Operation::Sub};
    instructions.insert(instructions.end(), {i1, i2, i3});
    return {i1.output, i2.output, i3.output};
}

glm::ivec2 generate_sub (std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v1, glm::ivec2 v2) {
    Instruction i1 = {v1.x, v2.x, current_register++, Operation::Sub};
    Instruction i2 = {v1.y, v2.y, current_register++, Operation::Sub};
    instructions.insert(instructions.end(), {i1, i2});
    return {i1.output, i2.output};
}

int generate_sub (std::vector<Instruction>& instructions, int& current_register, int v1, int v2) {
    Instruction i1 = {v1, v2, current_register++, Operation::Sub};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

int generate_add (std::vector<Instruction>& instructions, int& current_register, int v1, int v2) {
    Instruction i1 = {v1, v2, current_register++, Operation::Add};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

glm::ivec3 generate_add (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1, glm::ivec3 v2) {
    Instruction i1 = {v1.x, v2.x, current_register++, Operation::Add};
    Instruction i2 = {v1.y, v2.y, current_register++, Operation::Add};
    Instruction i3 = {v1.z, v2.z, current_register++, Operation::Add};
    instructions.insert(instructions.end(), {i1, i2, i3});
    return {i1.output, i2.output, i3.output};
}

int generate_mul (std::vector<Instruction>& instructions, int& current_register, int v1, int v2) {
    Instruction i1 = {v1, v2, current_register++, Operation::Mul};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

int generate_sqrt (std::vector<Instruction>& instructions, int& current_register, int v1) {
    Instruction i1 = {v1, -1, current_register++, Operation::Sqrt};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

int generate_min (std::vector<Instruction>& instructions, int& current_register, int v1, int v2) {
    Instruction i1 = {v1, v2, current_register++, Operation::Min};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

int generate_max (std::vector<Instruction>& instructions, int& current_register, int v1, int v2) {
    Instruction i1 = {v1, v2, current_register++, Operation::Max};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

glm::ivec2 generate_max (std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v1, glm::ivec2 v2) {
    Instruction i1 = {v1.x, v2.x, current_register++, Operation::Max};
    Instruction i2 = {v1.y, v2.y, current_register++, Operation::Max};
    instructions.insert(instructions.end(), {i1, i2});
    return {i1.output, i2.output};
}

glm::ivec3 generate_max (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1, glm::ivec3 v2) {
    Instruction i1 = {v1.x, v2.x, current_register++, Operation::Max};
    Instruction i2 = {v1.y, v2.y, current_register++, Operation::Max};
    Instruction i3 = {v1.z, v2.z, current_register++, Operation::Max};
    instructions.insert(instructions.end(), {i1, i2, i3});
    return {i1.output, i2.output, i3.output};
}

int generate_max_element (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1) {
    Instruction i1 = {v1.x, v1.y, current_register++, Operation::Max};
    Instruction i2 = {i1.output, v1.z, current_register++, Operation::Max};
    instructions.insert(instructions.end(), {i1, i2});
    return i2.output;
}

int generate_abs (std::vector<Instruction>& instructions, int& current_register, int v1) {
    Instruction i1 = {v1, -1, current_register++, Operation::Abs};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

glm::ivec3 generate_abs (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1) {
    Instruction i1 = {v1.x, -1, current_register++, Operation::Abs};
    Instruction i2 = {v1.y, -1, current_register++, Operation::Abs};
    Instruction i3 = {v1.z, -1, current_register++, Operation::Abs};
    instructions.insert(instructions.end(), {i1, i2, i3});
    return {i1.output, i2.output, i3.output};
}

int generate_sin (std::vector<Instruction>& instructions, int& current_register, int v1) {
    Instruction i1 = {v1, -1, current_register++, Operation::Sin};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

int generate_cos (std::vector<Instruction>& instructions, int& current_register, int v1) {
    Instruction i1 = {v1, -1, current_register++, Operation::Cos};
    instructions.insert(instructions.end(), {i1});
    return i1.output;
}

const char* make_op_name(Operation op){
    switch(op){
        case Operation::Add:
            return "Add";
        case Operation::Sub:
            return "Sub";
        case Operation::Mul:
            return "Mul";
        case Operation::Sqrt:
            return "Sqrt";
        case Operation::Min:
            return "Min";
        case Operation::Max:
            return "Max";
        case Operation::Abs:
            return "Abs";
        case Operation::Sin:
            return "Sin";
        case Operation::Cos:
            return "Cos";
        default:
            return "Unknown";
    }
}