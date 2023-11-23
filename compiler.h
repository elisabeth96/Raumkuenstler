//
// Created by elisabeth on 21.11.23.
//

#pragma once

#include <vector>
#include <map>
#include <functional>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

enum class Operation {
    None = 0,
    Add,
    Sub,
    Mul,
    Sqrt,
    Min,
    Max,
    Abs,
    Sin,
    Cos
};

// Representation of a single instruction used as an input for LLVM compiler
struct Instruction {
    int input1;
    int input2;
    int output;
    Operation operation;
};

std::function<double(glm::dvec3)> compile(std::vector<Instruction>& instructions, std::map<int, double>& constants);

// helper functions to create instructions
int generate_constant(std::map<int, double>& constants, int& current_register, double value);

int generate_length(std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v);

int generate_length(std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v);

glm::ivec3 generate_sub (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1, glm::ivec3 v2);

glm::ivec2 generate_sub (std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v1, glm::ivec2 v2);

int generate_sub (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

int generate_add (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

glm::ivec3 generate_add (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1, glm::ivec3 v2);

int generate_mul (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

int generate_sqrt (std::vector<Instruction>& instructions, int& current_register, int v1);

int generate_min (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

int generate_max (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

glm::ivec2 generate_max (std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v1, glm::ivec2 v2);

glm::ivec3 generate_max (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1, glm::ivec3 v2);

int generate_max_element (std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v1);

int generate_max_element (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1);

int generate_abs (std::vector<Instruction>& instructions, int& current_register, int v1);

glm::ivec2 generate_abs (std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v1);

glm::ivec3 generate_abs (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1);

int generate_sin (std::vector<Instruction>& instructions, int& current_register, int v1);

int generate_cos (std::vector<Instruction>& instructions, int& current_register, int v1);

const char* return_op_name(Operation op);

