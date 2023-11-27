#include <iostream>
#include <chrono>
#include <memory>

#include <polyscope/point_cloud.h>
#include <polyscope/surface_mesh.h>
#include <polyscope/curve_network.h>

#include <tbb/parallel_for.h>

#include "third_party/imnodes.h"
#include "implicit_meshing.h"
#include "editor.h"
#include "node.h"

namespace ps = polyscope;

// This is the function that will be called every frame
void callback() {

    static Editor editor;
    // Add dropdown menus to create nodes
    editor.draw_primitive_dropdown();
    ImGui::SameLine();
    editor.draw_operator_dropdown();
    ImGui::SameLine();
    editor.draw_math_dropdown();
    // Draw the nodes and handle links
    editor.draw();
    editor.handle_links();

    // Compute the mesh if there is a link to the output node and a re-mesh is needed
    if (!editor.m_remesh) {
        return;
    }
    if (editor.m_inputs[0][0].first == -1) {
        return;
    }
    std::vector<Instruction> instructions;
    std::map<int, double> constants;
    int current_register = 3;
    editor.m_nodes[0]->generate_instructions(instructions, current_register, constants);
    std::function<double(glm::dvec3)> f = compile(instructions, constants);
    auto start = std::chrono::high_resolution_clock::now();
    auto mesh = mesh_generator(f, 200);
    editor.m_remesh = false;
    auto end = std::chrono::high_resolution_clock::now();
    // print time in ms
    printf("Meshing took: %d microsec\n", (int) std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    ps::registerSurfaceMesh("my mesh", mesh.vertices, mesh.quads);
}


int main() {
    // warm up tbb
    std::vector<int> vec(1000000, 2);
    tbb::parallel_for(size_t(0), vec.size(), [&](size_t i) {
        vec[i] *= 2;
    });

    ps::options::buildGui = false;
    ps::options::groundPlaneMode = ps::GroundPlaneMode::ShadowOnly;
    ps::init();
    ImNodes::CreateContext();

    ps::state::userCallback = callback;

    ps::show();
    ImNodes::DestroyContext();

    return 0;
}
