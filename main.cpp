#include <iostream>

#include <polyscope/point_cloud.h>
#include <polyscope/surface_mesh.h>

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

double combine(double d1, double d2) {
    return std::min(d1, d2);
}

struct Edge {
    glm::ivec3 a;
    glm::ivec3 b;
    int idx;
};

std::vector<glm::dvec3> mesh_generator(std::function<double(glm::dvec3)> f, int n = 50) {
    glm::dvec3 lower{-3};
    glm::dvec3 upper{3};

    auto index_to_grid_point = [=](glm::dvec3 index) {
        return lower + index / (n - 1.0) * (upper - lower);
    };

    std::vector<double> grid(n * n * n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                glm::dvec3 p = index_to_grid_point(glm::dvec3(i, j, k));
                grid[i * n * n + j * n + k] = f(p);
            }
        }
    }
    std::vector<glm::dvec3> points;
    std::vector<Edge> edges = {
            {{1, 1, 0}, {1, 1, 1}, 2},
            {{1, 0, 1}, {1, 1, 1}, 1},
            {{0, 1, 1}, {1, 1, 1}, 0},
    };
    std::vector<std::pair<glm::dvec3, glm::dvec3>> all_edges = {
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
    std::vector<glm::dvec3> center_points(n * n * n, {0, 0, 0});
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1; j++) {
            for (int k = 0; k < n - 1; k++) {
                glm::dvec3 center_p = {0, 0, 0};
                int counter = 0;
                for (auto e: all_edges) {
                    glm::ivec3 index_p1 = {i + e.first.x, j + e.first.y, k + e.first.z};
                    glm::ivec3 index_p2 = {i + e.second.x, j + e.second.y, k + e.second.z};
                    double v1 = grid[index_p1.x * n * n + index_p1.y * n + index_p1.z];
                    double v2 = grid[index_p2.x * n * n + index_p2.y * n + index_p2.z];
                    if (v1 * v2 <= 0) {
                        glm::dvec3 p1 = index_to_grid_point(index_p1);
                        glm::dvec3 p2 = index_to_grid_point(index_p2);
                        center_p += p1 + v1 / (v1 - v2) * (p2 - p1);
                        counter++;
                    }
                }
                if (counter != 0) {
                    center_points[i * n * n + j * n + k] = center_p / (double) counter;
                }
            }
        }
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1; j++) {
            for (int k = 0; k < n - 1; k++) {
                glm::dvec3 p = center_points[i * n * n + j * n + k];
                for (auto e: edges) {
                    glm::ivec3 index_p1 = {i + e.a.x, j + e.a.y, k + e.a.z};
                    glm::ivec3 index_p2 = {i + e.b.x, j + e.b.y, k + e.b.z};
                    double v1 = grid[index_p1.x * n * n + index_p1.y * n + index_p1.z];
                    double v2 = grid[index_p2.x * n * n + index_p2.y * n + index_p2.z];
                    if (v1 * v2 <= 0) {
                        if (e.idx == 0) {
                            points.push_back(p);
                            points.push_back(center_points[i * n * n + j * n + k + 1]);
                            points.push_back(center_points[i * n * n + (j + 1) * n + k + 1]);
                            points.push_back(center_points[i * n * n + (j + 1) * n + k]);
                        }
                        if (e.idx == 1) {
                            points.push_back(p);
                            points.push_back(center_points[(i + 1) * n * n + j * n + k]);
                            points.push_back(center_points[(i + 1) * n * n + j * n + k + 1]);
                            points.push_back(center_points[i * n * n + j * n + k + 1]);
                        }
                        if (e.idx == 2) {
                            points.push_back(p);
                            points.push_back(center_points[i * n * n + (j + 1) * n + k]);
                            points.push_back(center_points[(i + 1) * n * n + (j + 1) * n + k]);
                            points.push_back(center_points[(i + 1) * n * n + j * n + k]);
                        }
                        if (v1 < 0) {
                            std::reverse(points.end()-4, points.end());
                        }
                    }
                }
            }
        }
    }
    return points;
}

void callback() {
    double t = ImGui::GetTime();
    std::vector<glm::dvec3> mesh_points = mesh_generator([=](glm::dvec3 v) {
        return combine(sphere(v, 0.25, 0.85 * glm::dvec3{std::cos(t), 0, std::sin(t)}), torus(v));
    }, 50);
    std::vector<std::array<int, 4>> faces;
    for (int i = 0; i < mesh_points.size() - 3; i++) {
        std::array<int, 4> a{i, ++i, ++i, ++i};
        faces.push_back(a);
    }
    ps::registerSurfaceMesh("my mesh", mesh_points, faces);
}


int main() {

    ps::init();


    ps::state::userCallback = callback;
    ps::show();

    return 0;
}
