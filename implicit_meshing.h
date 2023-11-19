//
// Created by elisabeth on 19.11.23.
//
#pragma once

#include <vector>
#include <array>
#include <glm/vec3.hpp>
#include <functional>

struct QuadMesh {
    std::vector<glm::dvec3> vertices;
    std::vector<std::array<int, 4>> quads;
};

// generate a mesh from an implicit function f with n^3 grid points
QuadMesh mesh_generator(std::function<double(glm::dvec3)> f, int n = 50);

