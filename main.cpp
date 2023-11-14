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

double box( glm::dvec3 p, glm::dvec3 b )
{
    glm::dvec3 q = abs(p) - b;
    return glm::length(max(q,0.0)) + std::min(std::max(q.x,std::max(q.y,q.z)),0.0);
}

double combine(double d1, double d2) {
    return std::min(d1, d2);
}

struct Edge {
    glm::ivec3 a;
    glm::ivec3 b;
    int idx;
};

std::pair<std::vector<glm::dvec3>, std::vector<std::array<int, 4>>>
mesh_generator(std::function<double(glm::dvec3)> f, int n = 50) {
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
    std::vector<std::array<int, 4>> faces;
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

    std::vector<int> index_points(n * n * n, -1);
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
                    points.push_back(center_p / (double) counter);
                    index_points[i * n * n + j * n + k] = (int) points.size() - 1;
                }
            }
        }
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1; j++) {
            for (int k = 0; k < n - 1; k++) {
                int index_p = index_points[i * n * n + j * n + k];
                for (auto e: edges) {
                    glm::ivec3 index_p1 = {i + e.a.x, j + e.a.y, k + e.a.z};
                    glm::ivec3 index_p2 = {i + e.b.x, j + e.b.y, k + e.b.z};
                    double v1 = grid[index_p1.x * n * n + index_p1.y * n + index_p1.z];
                    double v2 = grid[index_p2.x * n * n + index_p2.y * n + index_p2.z];
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
    auto mesh_box = mesh_generator([=](glm::dvec3 v) {
        return combine(box(v-moving_center, {0.2,0.2,0.2}), torus(v));
    }, 100);
    ps::registerSurfaceMesh("my mesh box", mesh_box.first, mesh_box.second);
}


int main() {

    ps::init();


    ps::state::userCallback = callback;
    ps::show();

    return 0;
}
