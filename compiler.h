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
    Add = 0,
    Sub,
    Mul,
    Sqrt,
    Min,
    Max,
    Abs
};

struct Instruction {
    int input1;
    int input2;
    int output;
    Operation operation;
};

std::function<double(glm::dvec3)> compile(std::vector<Instruction>& instructions, std::map<int, double>& constants);

int generate_length(std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v);

int generate_length(std::vector<Instruction>& instructions, int& current_register, glm::ivec2 v);

glm::ivec3 generate_sub (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1, glm::ivec3 v2);

int generate_sub (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

int generate_add (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

glm::ivec3 generate_add (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1, glm::ivec3 v2);

int generate_mul (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

int generate_sqrt (std::vector<Instruction>& instructions, int& current_register, int v1);

int generate_min (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

int generate_max (std::vector<Instruction>& instructions, int& current_register, int v1, int v2);

int generate_abs (std::vector<Instruction>& instructions, int& current_register, int v1);

glm::ivec3 generate_abs (std::vector<Instruction>& instructions, int& current_register, glm::ivec3 v1);

