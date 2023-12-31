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
        if (node == nullptr) {
            continue;
        }
        node->draw();
    }
    // check all possible link combinations and draw existing ones
    m_links.clear();
    int counter = 0;
    for (int i = 0; i < m_inputs.size(); ++i) {
        if (m_nodes[i] == nullptr) {
            continue;
        }
        for (int j = 0; j < m_inputs[i].size(); ++j) {
            if (m_inputs[i][j].node_id == -1) {
                continue;
            }
            m_links[counter] = {i, j};
            ImNodes::Link(counter++, OUTPUT_ATTRIBUTE_OFFSET + m_inputs[i][j].node_id,
                          INPUT_ATTRIBUTE_OFFSET + m_inputs[i][j].attribute_id);
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
                if (link.attribute_id == end_attr - INPUT_ATTRIBUTE_OFFSET && link.type == m_nodes[start_attr - OUTPUT_ATTRIBUTE_OFFSET].get()->m_output_type) {
                    link.node_id = start_attr - OUTPUT_ATTRIBUTE_OFFSET;
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

void Editor::draw_delete_button() {
    if (ImGui::Button("Delete")) {
        std::vector<int> nodes (ImNodes::NumSelectedNodes());
        ImNodes::GetSelectedNodes(nodes.data());
        for (auto it : nodes){
            for (auto& link : m_inputs){
                for (auto& slot : link){
                    if (slot.node_id == it){
                        slot.node_id = -1;
                    }
                }
            }
            m_nodes[it] = nullptr;
        }
        std::vector<int> links (ImNodes::NumSelectedLinks());
        ImNodes::GetSelectedLinks(links.data());
        for (auto link_id : links){
            auto index = m_links[link_id];
            m_inputs[index.first][index.second].node_id = -1;
        }
    }
}

Node *Editor::find_node(int node_id, int input_id) {
    int input_node_id = m_inputs[node_id][input_id].node_id;
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
        m_inputs.back().push_back({-1, m_current_input_id++, T::InputType[i]});
    }
}

int Editor::get_input_attribute_id(int node_id, int input_id) {
    return INPUT_ATTRIBUTE_OFFSET + m_inputs[node_id][input_id].attribute_id;
}

int Editor::get_output_attribute_id(int node_id) {
    return OUTPUT_ATTRIBUTE_OFFSET + node_id;
}
