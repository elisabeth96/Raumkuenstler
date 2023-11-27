//
// Created by elisabeth on 19.11.23.
//

#include "implicit_meshing.h"
#include <memory>
#include <glm/glm.hpp>

#include "third_party/probabilistic-quadrics.hh"
#include "third_party/hash_table7.hpp"

#include <tbb/parallel_for.h>

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

glm::dvec3 interpolate(std::pair<glm::dvec3, double> neg, std::pair<glm::dvec3, double> pos) {
    double val_neg = neg.second;
    double val_pos = pos.second;
    assert(val_neg <= 0);
    assert(val_pos >= 0);
    auto pt_neg = neg.first;
    auto pt_pos = pos.first;
    double t = val_neg / (val_neg - val_pos);
    glm::dvec3 p = pt_neg + (pt_pos - pt_neg) * t;
    return p;
}

glm::dvec3 find_point_on_surface(std::pair<glm::dvec3, double> neg, std::pair<glm::dvec3, double> pos,
                                 const std::function<double(glm::dvec3)> &f,
                                 int num_iterations) {
    assert(num_iterations > 0);
    const double threshold = 10e-5;
    for (int i = 0; i < num_iterations - 1; ++i) {
        glm::dvec3 p = interpolate(neg, pos);
        double val = f(p);
        if (std::abs(val) < threshold) {
            return p;
        }
        if (val < 0) {
            neg = {p, val};
        } else {
            pos = {p, val};
        }
    }

    // Return the best approximation if the loop completes without finding the exact point
    return interpolate(neg, pos);
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
    auto start_t = std::chrono::high_resolution_clock::now();
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
        if (abs(v) > 2 * glm::length(cell_upper - cell_lower) / 2.0) {
            continue;
        }
        // if the cell contains a zero-crossing, subdivide it into 8 smaller cells
        generate_children(grid_cells, cell);
    }
    auto end_t = std::chrono::high_resolution_clock::now();

    printf("Subdivision took: %d micro sec\n",
           (int) std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count());


    std::vector<int> edge_has_zero_crossing(grid.size() * 3, 0);
    std::vector<quadric> edge_quadrics(grid.size() * 3);
    std::vector<std::array<int, 4>> edge_faces(grid.size() * 3);

    std::vector<std::pair<glm::ivec3, double>> grid_array(grid.size());
    emhash7::HashMap<glm::ivec3, int, GridHash> grid_to_array;
    grid_to_array.reserve(grid.size());

    start_t = std::chrono::high_resolution_clock::now();
    {
        int i = 0;
        for (const auto &element: grid) {
            grid_array[i] = {element.first, element.second};
            grid_to_array[element.first] = i;
            ++i;
        }
    }
    end_t = std::chrono::high_resolution_clock::now();
    printf("Rehasing took %s micro sec\n",
           std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count()).c_str());

    start_t = std::chrono::high_resolution_clock::now();
    tbb::parallel_for(size_t(0), grid_array.size(), [&](size_t i) {
        auto [index1, v1] = grid_array[i];
        glm::dvec3 p1 = index_to_grid_point(index1);
        for (size_t j = 0; j < 3; ++j) {
            glm::ivec3 index2 = index1 + glm::ivec3(j == 0, j == 1, j == 2);
            auto it = grid.find(index2);
            if (it == grid.end()) {
                continue;
            }
            double v2 = it->second;
            if (v1 * v2 <= 0) {
                glm::dvec3 p2 = index_to_grid_point(index2);
                std::pair p1v1 = {p1, v1};
                std::pair p2v2 = {p2, v2};

                if (v1 > 0) {
                    std::swap(p1v1, p2v2);
                }
                auto zero_crossing = find_point_on_surface(p1v1, p2v2, f, 5);
                auto eq = quadric::probabilistic_plane_quadric(zero_crossing,
                                                               glm::normalize(gradient_f(zero_crossing)),
                                                               0.05, 0.05);
                edge_quadrics[i * 3 + j] = eq;
                edge_has_zero_crossing[i * 3 + j] = true;
            }
        }
    });
    end_t = std::chrono::high_resolution_clock::now();
    printf("Computing edge quadrics took: %d micro sec\n",
           (int) std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count());

    // generate vertex positions of the output mesh
    // for each voxel we compute a point if at least one of its edges contains a zero-crossing
    // the point is computed by minimizing a quadric error metric
    start_t = std::chrono::high_resolution_clock::now();

    std::vector<glm::dvec3> voxel_point(grid.size());
    std::vector<int> voxel_has_point(grid.size(), 0);

    // all edges must point from smaller to larger
    const std::vector<std::pair<glm::ivec3, glm::ivec3>> all_edges = {{{0, 0, 0}, {1, 0, 0}},
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
                                                                      {{0, 1, 1}, {1, 1, 1}}};


    tbb::parallel_for(size_t(0), grid_array.size(), [&](size_t i) {
        glm::ivec3 index = grid_array[i].first;
        size_t counter = 0;
        quadric vq;
        for (auto e: all_edges) {
            glm::ivec3 start = index + e.first;
            glm::ivec3 end = index + e.second;
            auto it = grid_to_array.find(start);
            if (it == grid_to_array.end()) {
                continue;
            }
            auto diff = end - start;
            assert(diff[0] >= 0);
            assert(diff[1] >= 0);
            assert(diff[2] >= 0);
            size_t k = diff[0] == 1 ? 0 : diff[1] == 1 ? 1 : 2;
            size_t j = 3 * it->second + k;
            if (edge_has_zero_crossing[j]) {
                vq += edge_quadrics[j];
                ++counter;
            }
        }
        if (counter > 0) {
            voxel_point[i] = vq.minimizer();
            voxel_has_point[i] = true;
        }
    });

    end_t = std::chrono::high_resolution_clock::now();
    printf("Computing positions took: %d micro sec\n",
           (int) std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count());


    int edge_indices[3][3] = {
            {0, 2, 1},
            {1, 0, 2},
            {2, 1, 0}
    };

    start_t = std::chrono::high_resolution_clock::now();
    std::vector<glm::dvec3> points;
    std::vector<int> vertex_map(voxel_point.size(), -1);
    {
        int i = 0;
        for (size_t j = 0; j < voxel_point.size(); ++j) {
            if (voxel_has_point[j]) {
                vertex_map[j] = i;
                points.push_back(voxel_point[j]);
                ++i;
            }
        }
    }
    end_t = std::chrono::high_resolution_clock::now();
    printf("Generating vertex map took: %d micro sec\n",
           (int) std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count());

    std::vector<std::array<int, 4>> quads;

    start_t = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < grid_array.size(); ++i) {
        glm::ivec3 a0 = grid_array[i].first;
        double v0 = grid_array[i].second;
        // iterate over the three edges starting at grid_idx
        for (auto [j, idx1, idx2]: edge_indices) {
            // check if the edge has a zero crossing
            if (!edge_has_zero_crossing[i * 3 + j]) {
                continue;
            }

            assert(voxel_has_point[i]);

            double v1 = grid_array[grid_to_array[a0 + glm::ivec3(j == 0, j == 1, j == 2)]].second;

            // generate quad
            glm::ivec3 a1 = a0;
            a1[idx1] -= 1;
            glm::ivec3 a2 = a0;
            a2[idx2] -= 1;
            glm::ivec3 a3 = a0;
            a3[idx1] -= 1;
            a3[idx2] -= 1;

            auto it1 = grid_to_array.find(a1);
            auto it2 = grid_to_array.find(a2);
            auto it3 = grid_to_array.find(a3);
            assert(it1 != grid_to_array.end());
            assert(it2 != grid_to_array.end());
            assert(it3 != grid_to_array.end());
            std::array<int, 4> quad{vertex_map[i], vertex_map[it1->second], vertex_map[it3->second],
                                    vertex_map[it2->second]};
            // there are 7 relevant cases:
            // v0 == 0 && v1 == 0 -> ambiguous, lets orient toward v0 for now
            // v0 == 0 && v1 < 0 -> orient toward v0
            // v1 == 0 && v0 > 0 -> orient toward v0
            // v0 > 0 && v1 < 0 -> orient toward v0
            // v0 == 0 && v1 > 0 -> orient toward v1
            // v1 == 0 && v0 < 0 -> orient toward v1
            // v1 > 0 && v0 < 0 -> orient toward v1
            // by default the face is oriented towards v0
            if (v0 == 0 && v1 > 0 || v1 == 0 && v0 < 0 || v1 > 0 && v0 < 0) {
                std::reverse(quad.begin(), quad.end());
            }
            assert(quad[0] != -1);
            assert(quad[1] != -1);
            assert(quad[2] != -1);
            assert(quad[3] != -1);
            quads.push_back(quad);
        }
    }
    end_t = std::chrono::high_resolution_clock::now();
    printf("Generating quads took: %d micro sec\n",
           (int) std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count());

    return {std::move(points), std::move(quads)};
}
