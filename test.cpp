//
// Created by elisabeth on 21.11.23.
//
#include "compiler.h"
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <chrono>

double sphere(double x, double y, double z) {
    return std::sqrt(x * x + y * y + z * z) - 1;
}

void test(std::function<double(double, double, double)> f, const std::vector<std::array<double, 3>> &inputs,
          std::vector<double> &results) {
    for (int i = 0; i < results.size(); ++i) {
        double x = inputs[i][0];
        double y = inputs[i][1];
        double z = inputs[i][2];
        results[i] = f(x, y, z);
    }
}

int main() {
    std::vector<Instruction> instructions;
    std::map<int, double> constants;
    constants[6] = 1;
    constants[3] = 0;
    constants[4] = 0;
    constants[5] = 0;
    int x = 0;
    int y = 1;
    int z = 2;
    std::vector<int> center = {3, 4, 5};
    std::vector<int> radius = {6};
    int current_register = 7;
    Instruction i0 = {0, center[0], current_register++, Operation::Sub};
    Instruction i1 = {1, center[1], current_register++, Operation::Sub};
    Instruction i2 = {2, center[2], current_register++, Operation::Sub};
    Instruction i3 = {i0.output, i0.output, current_register++, Operation::Mul};
    Instruction i4 = {i1.output, i1.output, current_register++, Operation::Mul};
    Instruction i5 = {i2.output, i2.output, current_register++, Operation::Mul};
    Instruction i6 = {i3.output, i4.output, current_register++, Operation::Add};
    Instruction i7 = {i5.output, i6.output, current_register++, Operation::Add};
    Instruction i8 = {i7.output, -1, current_register++, Operation::Sqrt};
    Instruction i9 = {i8.output, radius[0], current_register++, Operation::Sub};
    instructions.insert(instructions.end(), {i0, i1, i2, i3, i4, i5, i6, i7, i8, i9});

    std::cout << std::endl;
    auto start_compile = std::chrono::high_resolution_clock::now();
    std::function<double(double, double, double)> f = compile(instructions, constants);
    auto end_compile = std::chrono::high_resolution_clock::now();
    printf("Compile: %f ms\n", std::chrono::duration<double, std::milli>(end_compile - start_compile).count());

    std::vector<std::array<double, 3>> inputs(1'000'000);
    std::vector<double> results1(1'000'000);
    std::vector<double> results2(1'000'000);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1, 1);
    for (auto &input: inputs) {
        input[0] = dis(gen);
        input[1] = dis(gen);
        input[2] = dis(gen);
    }

    auto start = std::chrono::high_resolution_clock::now();
    test(sphere, inputs, results1);
    auto end = std::chrono::high_resolution_clock::now();

    printf("C++ Sphere: %f ms\n", std::chrono::duration<double, std::milli>(end - start).count());

    start = std::chrono::high_resolution_clock::now();
    test(f, inputs, results2);
    end = std::chrono::high_resolution_clock::now();

    printf("LLVM Sphere: %f ms\n", std::chrono::duration<double, std::milli>(end - start).count());

    // check results are the same
    for (int i = 0; i < results1.size(); ++i) {
        if (std::abs(results1[i] - results2[i]) > 1e-6) {
            std::cout << "Results differ at index " << i << std::endl;
            std::cout << "C++: " << results1[i] << std::endl;
            std::cout << "LLVM: " << results2[i] << std::endl;
            return 1;
        }
    }

    return 0;
}


