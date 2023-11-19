#include <iostream>
#include <chrono>
#include <memory>

#include <polyscope/point_cloud.h>
#include <polyscope/surface_mesh.h>
#include <polyscope/curve_network.h>

#include "imnodes.h"
#include "implicit_meshing.h"
#include "editor.h"
#include "node.h"

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

void callback() {/*

    double t = ImGui::GetTime();
    glm::dvec3 moving_center = 0.85 * glm::dvec3{std::cos(t), 0, std::sin(t)};
    //auto mesh_circle = mesh_generator([=](glm::dvec3 v) {
    //    return combine(sphere(v, 0.25, moving_center), torus(v));
    //}, 100);
    auto start = std::chrono::high_resolution_clock::now();
    auto mesh_box = mesh_generator([=](glm::dvec3 v) {
        return opSmoothUnion(box(v - moving_center, {0.2, 0.2, 0.2}), torus(v), 0.3);
    }, 200);
    auto end = std::chrono::high_resolution_clock::now();
    // print time in ms
    printf("Time taken: %d ms\n", (int) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    ps::registerSurfaceMesh("my mesh box", mesh_box.first, mesh_box.second);
    */

    static Editor editor;

    static int selectedNode = -1;
    ImGui::PushItemWidth(120);
    if (ImGui::BeginCombo("##combo", "Add Node")) {
        if (ImGui::Selectable("Sphere", selectedNode == 0)) {
            selectedNode = 0;
            editor.nodes.push_back(
                    std::make_unique<SphereNode>(&editor));
        }
        if (ImGui::Selectable("Scalar", selectedNode == 1)) {
            selectedNode = 1;
            editor.nodes.push_back(std::make_unique<ScalarNode>(&editor));
        }
        if (ImGui::Selectable("Point", selectedNode == 2)) {
            selectedNode = 2;
            editor.nodes.push_back(std::make_unique<PointNode>(&editor));
        }

        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    editor.draw();
    int start_attr, end_attr;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        if (start_attr == end_attr) {
            return;
        }
        auto it = std::find_if(editor.links.begin(), editor.links.end(), [=](auto pair) {
            return pair.second == end_attr;
        });
        if (it != editor.links.end()) {
            editor.links.erase(it);
        }
        editor.links.emplace_back(start_attr, end_attr);
    }
}


int main() {
    ps::options::buildGui = false;

    ps::init();
    ImNodes::CreateContext();


    ps::state::userCallback = callback;
    /*glm::dvec3 center = {0, 0, 0};
    auto mesh_circle = mesh_generator([=](glm::dvec3 v) {
        return sphere(v, 1, center);
    }, 50);
    ps::registerSurfaceMesh("my mesh box", mesh_circle.first, mesh_circle.second);*/

    ps::show();
    ImNodes::DestroyContext();

    return 0;
}
