//
// Created by elisabeth on 19.11.23.
//

#include "implicit_meshing.h"
#include <memory>
#include <glm/glm.hpp>

#include "third_party/probabilistic-quadrics.hh"
#include "third_party/hash_table7.hpp"

using glm_trait = pq::math<double, glm::dvec3, glm::dvec3, glm::dmat3>;
using quadric = pq::quadric<glm_trait>;

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

QuadMesh mesh_generator(std::function<double(glm::dvec3)> f, int n) {
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

    /* used for debugging
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
    };*/

    // stack of grid cells used in recursive subdivision algorithm
    std::vector<GridCell> grid_cells;
    grid_cells.push_back({{0, 0, 0},
                          {n, n, n}});
    emhash7::HashMap<glm::ivec3, double, GridHash> grid;

    // subdivide cells that contain zero-crossings
    while (!grid_cells.empty()) {
        GridCell cell = grid_cells.back();
        grid_cells.pop_back();
        glm::ivec3 grid_size = cell.second - cell.first;
        // if the cell is too small to divide it again, just evaluate the function at the lower corner
        if (grid_size.x == 1 || grid_size.y == 1 || grid_size.z == 1) {
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
        // if the cell does not contain a zero-crossing, skip the cell
        glm::dvec3 cell_lower = index_to_grid_point(cell.first);
        glm::dvec3 cell_upper = index_to_grid_point(cell.second);
        double v = f((cell_upper + cell_lower) / 2.0);
        if (abs(v) > 1.5 * glm::length(cell_upper - cell_lower)/2.0) {
            continue;
        }
        // if the cell contains a zero-crossing, subdivide it into 8 smaller cells
        generate_children(grid_cells, cell);
    }

    std::vector<glm::dvec3> points;
    std::vector<std::array<int, 4>> faces;
    std::vector<Edge> edges = {{{1, 1, 0}, {1, 1, 1}, 2},
                               {{1, 0, 1}, {1, 1, 1}, 1},
                               {{0, 1, 1}, {1, 1, 1}, 0},};
    std::vector<std::pair<glm::ivec3, glm::ivec3>> all_edges = {{{0, 0, 0}, {1, 0, 0}},
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
                                                                {{0, 1, 1}, {1, 1, 1}},};

    std::vector<int> index_points(n * n * n, -1);

    // generate vertex positions of the output mesh
    // for each voxel we compute a point if at least one of its edges contains a zero-crossing
    // the point is computed by minimizing a quadric error metric
    for (const auto &element: grid) {
        glm::ivec3 index = element.first;
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
                q += quadric::probabilistic_plane_quadric(zero_crossing, glm::normalize(gradient_f(zero_crossing)),
                                                          0.05, 0.05);
            }
        }
        if (counter != 0) {
            points.push_back(q.minimizer());
            index_points[index.x * n * n + index.y * n + index.z] = (int) points.size() - 1;
        }
    }

    // generate faces of the output mesh by connecting the corresponding points
    for (const auto &element: grid) {
        glm::ivec3 index = element.first;
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
