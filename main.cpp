#include <iostream>
#include <chrono>

#include <polyscope/point_cloud.h>
#include <polyscope/surface_mesh.h>
#include <polyscope/curve_network.h>

#include "probabilistic-quadrics.hh"
#include "hash_table7.hpp"

using glm_trait = pq::math<double, glm::dvec3, glm::dvec3, glm::dmat3>;
using quadric = pq::quadric<glm_trait>;

namespace ps = polyscope;

double sphere(glm::dvec3 p, double r = 0.3, glm::dvec3 c = {0, 0, 0}) {
    return glm::distance(p, c) - r;
}

double torus(glm::dvec3 p) {
    double r1 = 0.5;
    double r2 = 0.25;
    double x = sqrt(p.x * p.x + p.z * p.z) - r1;
    return sqrt(x * x + p.y * p.y) - r2;
}

double box(glm::dvec3 p, glm::dvec3 b) {
    glm::dvec3 q = abs(p) - b;
    return glm::length(max(q, 0.0)) + std::min(std::max(q.x, std::max(q.y, q.z)), 0.0);
}

double combine(double d1, double d2) {
    return std::min(d1, d2);
}

double opSmoothUnion(double d1, double d2, double k) {
    double h = glm::clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return glm::mix(d2, d1, h) - k * h * (1.0 - h);
}

// Edge between two adjaccent grid points a and b
// idx shows the axis of the edge (0 -> x, 1 -> y, 2 -> z)
struct Edge {
    glm::ivec3 a;
    glm::ivec3 b;
    int idx;
};

struct GridHash {
    size_t operator()(const glm::ivec3 &v) const {
        // Use std::hash for individual components and combine them
        size_t seed = 0;
        seed ^= std::hash<int>()(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

// GridCell represents a range of grid points in 3D space.
// The first vector defines the inclusive lower-left front grid index,
// while the second vector defines the exclusive upper-right back grid index.
using GridCell = std::pair<glm::ivec3, glm::ivec3>;

// Split current grid cell intro 8 smaller cells
void generate_children(std::vector<GridCell> &grid_cells, const GridCell &current) {
    glm::ivec3 min_coord = current.first;
    glm::ivec3 max_coord = current.second;

    // Ensure the cell dimensions are valid
    if (max_coord.x <= min_coord.x || max_coord.y <= min_coord.y || max_coord.z <= min_coord.z) {
        throw std::invalid_argument("Invalid cell dimensions");
    }

    // Calculate the size of each child cell
    glm::ivec3 cell_size = (max_coord - min_coord) / 2;

    // Ensure each child cell will have a positive size
    assert(cell_size.x > 0 && cell_size.y > 0 && cell_size.z > 0);

    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z) {
                glm::ivec3 child_min = min_coord + cell_size * glm::ivec3(x, y, z);
                glm::ivec3 child_max = child_min + cell_size;

                // Adjust the max boundary for the last cell in each dimension
                if (x == 1) child_max.x = max_coord.x;
                if (y == 1) child_max.y = max_coord.y;
                if (z == 1) child_max.z = max_coord.z;

                GridCell child_cell(child_min, child_max);
                grid_cells.push_back(child_cell);
            }
        }
    }
}

std::pair<std::vector<glm::dvec3>, std::vector<std::array<int, 4>>>
mesh_generator(std::function<double(glm::dvec3)> f, int n = 50) {
    glm::dvec3 lower{-3};
    glm::dvec3 upper{3};

    auto index_to_grid_point = [=](glm::dvec3 index) {
        return lower + index / (n - 1.0) * (upper - lower);
    };

    auto gradient_f = [=](glm::dvec3 p) -> glm::dvec3 {
        double eps = 10e-5;
        double dx = (f({p.x + eps, p.y, p.z}) - f({p.x - eps, p.y, p.z})) / (2 * eps);
        double dy = (f({p.x, p.y + eps, p.z}) - f({p.x, p.y - eps, p.z})) / (2 * eps);
        double dz = (f({p.x, p.y, p.z + eps}) - f({p.x, p.y, p.z - eps})) / (2 * eps);
        return {dx, dy, dz};
    };

    auto draw_grid_cell = [=](GridCell cell) {
        glm::vec3 lower = index_to_grid_point(cell.first);
        glm::vec3 upper = index_to_grid_point(cell.second);
        std::vector<glm::vec3> pts{
                // Bottom square
                glm::vec3{lower[0], lower[1], lower[2]}, glm::vec3{upper[0], lower[1], lower[2]},
                glm::vec3{upper[0], lower[1], lower[2]}, glm::vec3{upper[0], lower[1], upper[2]},
                glm::vec3{upper[0], lower[1], upper[2]}, glm::vec3{lower[0], lower[1], upper[2]},
                glm::vec3{lower[0], lower[1], upper[2]}, glm::vec3{lower[0], lower[1], lower[2]},

                // Top square
                glm::vec3{lower[0], upper[1], lower[2]}, glm::vec3{upper[0], upper[1], lower[2]},
                glm::vec3{upper[0], upper[1], lower[2]}, glm::vec3{upper[0], upper[1], upper[2]},
                glm::vec3{upper[0], upper[1], upper[2]}, glm::vec3{lower[0], upper[1], upper[2]},
                glm::vec3{lower[0], upper[1], upper[2]}, glm::vec3{lower[0], upper[1], lower[2]},

                // Connecting lines between top and bottom squares
                glm::vec3{lower[0], lower[1], lower[2]}, glm::vec3{lower[0], upper[1], lower[2]},
                glm::vec3{upper[0], lower[1], lower[2]}, glm::vec3{upper[0], upper[1], lower[2]},
                glm::vec3{upper[0], lower[1], upper[2]}, glm::vec3{upper[0], upper[1], upper[2]},
                glm::vec3{lower[0], lower[1], upper[2]}, glm::vec3{lower[0], upper[1], upper[2]}
        };

        std::vector<std::array<size_t, 2>> lines
                = {{0,  1},
                   {2,  3},
                   {4,  5},
                   {6,  7},
                   {8,  9},
                   {10, 11},
                   {12, 13},
                   {14, 15},
                   {16, 17},
                   {18, 19},
                   {20, 21},
                   {22, 23}};

        static size_t id = 0;
        auto bb_lines = ps::registerCurveNetwork("bounding_box" + std::to_string(id++), pts, lines);
        bb_lines->setRadius(0.003);
    };

    // stack of grid cells used in recursive subdivision algorithm
    std::vector<GridCell> grid_cells;
    grid_cells.push_back({{0, 0, 0},
                          {n, n, n}});
    emhash7::HashMap<glm::ivec3, double, GridHash> grid;

    while (!grid_cells.empty()) {
        GridCell cell = grid_cells.back();
        grid_cells.pop_back();
        //draw_grid_cell(cell);
        glm::ivec3 grid_size = cell.second - cell.first;
        // needs to be changed if we want to support nxmxk grid
        if (grid_size.x == 1 || grid_size.y == 1 || grid_size.z == 1) {
            // iterate over all grid points in the cell
            for (int i = cell.first.x; i < cell.second.x; ++i) {
                for (int j = cell.first.y; j < cell.second.y; ++j) {
                    for (int k = cell.first.z; k < cell.second.z; ++k) {
                        glm::ivec3 index = {i, j, k};
                        grid[index] = f(index_to_grid_point(index));
                    }
                }
            }
            continue;
        }
        glm::ivec3 center_index = (cell.second + cell.first) / 2;
        double v = f(index_to_grid_point(center_index));
        if (abs(v) > 1.5*std::max(glm::length(index_to_grid_point(cell.second) - index_to_grid_point(center_index)),
                              glm::length(index_to_grid_point(cell.first) - index_to_grid_point(center_index)))) {
            continue;
        }
        generate_children(grid_cells, cell);
    }

    std::vector<glm::dvec3> points;
    std::vector<std::array<int, 4>> faces;
    std::vector<Edge> edges = {
            {{1, 1, 0}, {1, 1, 1}, 2},
            {{1, 0, 1}, {1, 1, 1}, 1},
            {{0, 1, 1}, {1, 1, 1}, 0},
    };
    std::vector<std::pair<glm::ivec3, glm::ivec3>> all_edges = {
            {{0, 0, 0}, {1, 0, 0}},
            {{0, 0, 0}, {0, 1, 0}},
            {{0, 0, 0}, {0, 0, 1}},
            {{1, 0, 0}, {1, 1, 0}},
            {{1, 0, 0}, {1, 0, 1}},
            {{0, 1, 0}, {1, 1, 0}},
            {{0, 1, 0}, {0, 1, 1}},
            {{0, 0, 1}, {1, 0, 1}},
            {{0, 0, 1}, {0, 1, 1}},
            {{1, 1, 0}, {1, 1, 1}},
            {{1, 0, 1}, {1, 1, 1}},
            {{0, 1, 1}, {1, 1, 1}},
    };

    std::vector<int> index_points(n * n * n, -1);

    for (const auto& element : grid) {
        glm::ivec3 index = element.first;
        double v = element.second;
        quadric q;
        int counter = 0;
        for (auto e: all_edges) {
            glm::ivec3 index_p1 = index + e.first;
            glm::ivec3 index_p2 = index + e.second;
            auto it1 = grid.find(index_p1);
            auto it2 = grid.find(index_p2);
            if (it1 == grid.end() || it2 == grid.end()) {
                continue;
            }
            double v1 = it1->second;
            double v2 = it2->second;
            if (v1 * v2 <= 0) {
                glm::dvec3 p1 = index_to_grid_point(index_p1);
                glm::dvec3 p2 = index_to_grid_point(index_p2);
                glm::dvec3 zero_crossing = p1 + v1 / (v1 - v2) * (p2 - p1);
                counter++;
                q += quadric::probabilistic_plane_quadric(zero_crossing,
                                                          glm::normalize(gradient_f(zero_crossing)), 0.05,
                                                          0.05);
            }
        }
        if (counter != 0) {
            points.push_back(q.minimizer());
            index_points[index.x * n * n + index.y * n + index.z] = (int) points.size() - 1;
        }
    }

    for (const auto& element: grid) {
        glm::ivec3 index = element.first;
        double v = element.second;
        int i = index.x;
        int j = index.y;
        int k = index.z;
        int index_p = index_points[i * n * n + j * n + k];
        if (index_p == -1) {
            continue;
        }
        for (auto e: edges) {
            glm::ivec3 index_p1 = index + e.a;
            glm::ivec3 index_p2 = index + e.b;
            auto it1 = grid.find(index_p1);
            auto it2 = grid.find(index_p2);
            if (it1 == grid.end() || it2 == grid.end()) {
                continue;
            }
            double v1 = it1->second;
            double v2 = it2->second;
            if (v1 * v2 <= 0) {
                std::array<int, 4> face{};
                if (e.idx == 0) {
                    face[0] = index_p;
                    face[1] = index_points[i * n * n + j * n + k + 1];
                    face[2] = index_points[i * n * n + (j + 1) * n + k + 1];
                    face[3] = index_points[i * n * n + (j + 1) * n + k];
                }
                if (e.idx == 1) {
                    face[0] = index_p;
                    face[1] = index_points[(i + 1) * n * n + j * n + k];
                    face[2] = index_points[(i + 1) * n * n + j * n + k + 1];
                    face[3] = index_points[i * n * n + j * n + k + 1];
                }
                if (e.idx == 2) {
                    face[0] = index_p;
                    face[1] = index_points[i * n * n + (j + 1) * n + k];
                    face[2] = index_points[(i + 1) * n * n + (j + 1) * n + k];
                    face[3] = index_points[(i + 1) * n * n + j * n + k];
                }
                if (v1 < 0) {
                    std::reverse(face.begin(), face.end());
                }
                faces.push_back(face);
            }
        }
    }
    return {points, faces};
}

void callback() {
    double t = ImGui::GetTime();
    glm::dvec3 moving_center = 0.85 * glm::dvec3{std::cos(t), 0, std::sin(t)};
    /*auto mesh_circle = mesh_generator([=](glm::dvec3 v) {
        return combine(sphere(v, 0.25, moving_center), torus(v));
    }, 100);*/
    auto start = std::chrono::high_resolution_clock::now();
    auto mesh_box = mesh_generator([=](glm::dvec3 v) {
        return opSmoothUnion(box(v - moving_center, {0.2, 0.2, 0.2}), torus(v), 0.3);
    }, 200);
    auto end = std::chrono::high_resolution_clock::now();
    // print time in ms
    printf("Time taken: %d ms\n", (int) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    ps::registerSurfaceMesh("my mesh box", mesh_box.first, mesh_box.second);
}


int main() {

    ps::init();


    ps::state::userCallback = callback;
    /*glm::dvec3 center = {0, 0, 0};
    auto mesh_circle = mesh_generator([=](glm::dvec3 v) {
        return sphere(v, 1, center);
    }, 50);
    ps::registerSurfaceMesh("my mesh box", mesh_circle.first, mesh_circle.second);*/

    ps::show();

    return 0;
}
