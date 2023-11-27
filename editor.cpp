//
// Created by elisabeth on 19.11.23.
//

#include "editor.h"
#include "third_party/imnodes.h"
#include <algorithm>

// Every Editor contains an OutputNode
Editor::Editor() {
    add_node<OutputNode>();
}

void Editor::draw() {
    ImNodes::BeginNodeEditor();
    // draw all nodes that currenty exist
    for (auto &node: m_nodes) {
        node->draw();
    }
    // check all possible link combinations and draw existing ones
    int counter = 0;
    for (int i = 0; i < m_inputs.size(); ++i) {
        for (int j = 0; j < m_inputs[i].size(); ++j) {
            if (m_inputs[i][j].first == -1) {
                continue;
            }
            ImNodes::Link(counter++, OUTPUT_ATTRIBUTE_OFFSET + m_inputs[i][j].first,
                          INPUT_ATTRIBUTE_OFFSET + m_inputs[i][j].second);
        }
    }
    ImNodes::EndNodeEditor();
}

void Editor::handle_links() {
    int start_attr, end_attr;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        if (start_attr == end_attr) {
            return;
        }
        for (auto &input: m_inputs) {
            for (auto &link: input) {
                if (link.second == end_attr - INPUT_ATTRIBUTE_OFFSET) {
                    link.first = start_attr - OUTPUT_ATTRIBUTE_OFFSET;
                    m_remesh = true;
                    return;
                }
            }
        }
    }
}

void Editor::draw_primitive_dropdown() {
    static int selected_node = -1;
    ImGui::PushItemWidth(120);
    if (ImGui::BeginCombo("##combo", "Primitive")) {
        if (ImGui::Selectable("Sphere", selected_node == 0)) {
            selected_node = 0;
            add_node<SphereNode>();
        }
        if (ImGui::Selectable("Torus", selected_node == 1)) {
            selected_node = 1;
            add_node<TorusNode>();
        }
        if (ImGui::Selectable("Box", selected_node == 2)) {
            selected_node = 2;
            add_node<BoxNode>();
        }
        if (ImGui::Selectable("Cylinder", selected_node == 3)) {
            selected_node = 3;
            add_node<CylinderNode>();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
}

void Editor::draw_operator_dropdown() {
    static int selected_node = -1;
    ImGui::PushItemWidth(120);
    if (ImGui::BeginCombo("##combo1", "Operator")) {
        if (ImGui::Selectable("Union", selected_node == 0)) {
            selected_node = 0;
            add_node<UnionNode>();
        }
        if (ImGui::Selectable("Smooth Union", selected_node == 1)) {
            selected_node = 1;
            add_node<SmoothUnionNode>();
        }
        if (ImGui::Selectable("Intersection", selected_node == 2)) {
            selected_node = 2;
            add_node<IntersectionNode>();
        }
        if (ImGui::Selectable("Subtraction", selected_node == 3)) {
            selected_node = 3;
            add_node<SubtractionNode>();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
}

void Editor::draw_math_dropdown() {
    static int selected_node = -1;
    ImGui::PushItemWidth(120);
    if (ImGui::BeginCombo("##combo2", "Math")) {
        if (ImGui::Selectable("Scalar", selected_node == 0)) {
            selected_node = 0;
            add_node<ScalarNode>();
        }
        if (ImGui::Selectable("Point", selected_node == 1)) {
            selected_node = 1;
            add_node<PointNode>();
        }
        if (ImGui::Selectable("Time", selected_node == 2)) {
            selected_node = 2;
            add_node<TimeNode>();
        }
        if (ImGui::Selectable("Sine", selected_node == 3)) {
            selected_node = 3;
            add_node<UnaryOpNode, Operation::Sin>();
        }
        if (ImGui::Selectable("Cosine", selected_node == 4)) {
            selected_node = 4;
            add_node<UnaryOpNode, Operation::Cos>();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
}

Node *Editor::find_node(int node_id, int input_id) {
    int input_node_id = m_inputs[node_id][input_id].first;
    if (input_node_id == -1) {
        return nullptr;
    }
    return m_nodes[input_node_id].get();
}

template<class T, Operation op>
void Editor::add_node() {
    if constexpr (op == Operation::None) {
        m_nodes.push_back(std::make_unique<T>(this, m_nodes.size()));
    } else {
        m_nodes.push_back(std::make_unique<T>(this, m_nodes.size(), op));
    }
    m_inputs.emplace_back();
    for (int i = 0; i < m_nodes.back()->m_num_inputs; ++i) {
        m_inputs.back().emplace_back(-1, m_current_input_id++);
    }
}

int Editor::get_input_attribute_id(int node_id, int input_id) {
    return INPUT_ATTRIBUTE_OFFSET + m_inputs[node_id][input_id].second;
}

int Editor::get_output_attribute_id(int node_id) {
    return OUTPUT_ATTRIBUTE_OFFSET + node_id;
}
