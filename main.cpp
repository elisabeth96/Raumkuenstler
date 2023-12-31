#include <iostream>
#include <chrono>
#include <memory>

#include <polyscope/point_cloud.h>
#include <polyscope/surface_mesh.h>
#include <polyscope/curve_network.h>

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
    // Draw delete button
    ImGui::SameLine();
    editor.draw_delete_button();

    // Draw the nodes and handle links
    editor.draw();
    editor.handle_links();

    // Compute the mesh if there is a link to the output node and a re-mesh is needed
    if (!editor.m_remesh) {
        return;
    }
    if (editor.m_inputs[0][0].node_id == -1) {
        return;
    }
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Instruction> instructions;
    std::map<int, double> constants;
    int current_register = 3;
    editor.m_nodes[0]->generate_instructions(instructions, current_register, constants);
    std::function<double(glm::dvec3)> f = compile(instructions, constants);
    auto mesh = mesh_generator(f, 200);
    editor.m_remesh = false;
    auto end = std::chrono::high_resolution_clock::now();
    // print time in ms
    printf("Time taken: %d ms\n", (int) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    ps::registerSurfaceMesh("my mesh", mesh.vertices, mesh.quads);
}


int main() {
    ps::options::buildGui = false;
    ps::options::groundPlaneMode = ps::GroundPlaneMode::ShadowOnly;
    ps::init();
    ImNodes::CreateContext();

    ps::state::userCallback = callback;

    ps::show();
    ImNodes::DestroyContext();

    return 0;
}
