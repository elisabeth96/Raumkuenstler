//
// Created by elisabeth on 19.11.23.
//

#include "editor.h"
#include "imnodes.h"
#include <algorithm>

Editor::Editor() {
    add_node<OutputNode>();
}

void Editor::draw() {
    ImNodes::BeginNodeEditor();
    for (auto &node: m_nodes) {
        node->draw();
    }
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
                if (link.second == end_attr - INPUT_ATTRIBUTE_OFFSET){
                    link.first = start_attr - OUTPUT_ATTRIBUTE_OFFSET;
                    m_remesh = true;
                    return;
                }
            }
        }
    }
}

void Editor::draw_node_dropdown() {
    static int selectedNode = -1;
    ImGui::PushItemWidth(120);
    if (ImGui::BeginCombo("##combo", "Add Node")) {
        if (ImGui::Selectable("Sphere", selectedNode == 0)) {
            selectedNode = 0;
            add_node<SphereNode>();
        }
        if (ImGui::Selectable("Scalar", selectedNode == 1)) {
            selectedNode = 1;
            add_node<ScalarNode>();
        }
        if (ImGui::Selectable("Point", selectedNode == 2)) {
            selectedNode = 2;
            add_node<PointNode>();
        }
        if (ImGui::Selectable("Torus", selectedNode == 3)) {
            selectedNode = 3;
            add_node<TorusNode>();
        }
        if (ImGui::Selectable("Box", selectedNode == 4)) {
            selectedNode = 4;
            add_node<BoxNode>();
        }
        if (ImGui::Selectable("Union", selectedNode == 5)) {
            selectedNode = 5;
            add_node<UnionNode>();
        }
        if (ImGui::Selectable("Smooth Union", selectedNode == 6)) {
            selectedNode = 6;
            add_node<SmoothUnionNode>();
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

template<class T>
void Editor::add_node() {
    m_nodes.push_back(std::make_unique<T>(this, m_nodes.size()));
    m_inputs.emplace_back();
    for (int i = 0; i < m_nodes.back()->m_num_inputs; ++i) {
        m_inputs.back().emplace_back(-1, m_current_input_id++);
    }
}

int Editor::get_input_attribute_id(int node_id, int input_id) {
    return INPUT_ATTRIBUTE_OFFSET+ m_inputs[node_id][input_id].second;
}

int Editor::get_output_attribute_id(int node_id) {
    return OUTPUT_ATTRIBUTE_OFFSET+ node_id;
}
