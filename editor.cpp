//
// Created by elisabeth on 19.11.23.
//

#include "editor.h"
#include "imnodes.h"
#include <algorithm>

void Editor::draw() {
    ImNodes::BeginNodeEditor();
    output.draw();
    for (auto &node: nodes) {
        node->draw();
    }
    for (int i = 0; i < links.size(); ++i) {
        ImNodes::Link(i, links[i].first, links[i].second);
    }
    ImNodes::EndNodeEditor();
}

void Editor::handle_links() {
    int start_attr, end_attr;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        if (start_attr == end_attr) {
            return;
        }
        auto it = std::find_if(links.begin(), links.end(), [=](auto pair) {
            return pair.second == end_attr;
        });
        if (it != links.end()) {
            links.erase(it);
        }
        links.emplace_back(start_attr, end_attr);
    }
}

void Editor::draw_node_dropdown() {
    static int selectedNode = -1;
    ImGui::PushItemWidth(120);
    if (ImGui::BeginCombo("##combo", "Add Node")) {
        if (ImGui::Selectable("Sphere", selectedNode == 0)) {
            selectedNode = 0;
            nodes.push_back(std::make_unique<SphereNode>(this));
        }
        if (ImGui::Selectable("Scalar", selectedNode == 1)) {
            selectedNode = 1;
            nodes.push_back(std::make_unique<ScalarNode>(this));
        }
        if (ImGui::Selectable("Point", selectedNode == 2)) {
            selectedNode = 2;
            nodes.push_back(std::make_unique<PointNode>(this));
        }
        if (ImGui::Selectable("Torus", selectedNode == 3)) {
            selectedNode = 3;
            nodes.push_back(std::make_unique<TorusNode>(this));
        }
        if (ImGui::Selectable("Box", selectedNode == 4)) {
            selectedNode = 4;
            nodes.push_back(std::make_unique<BoxNode>(this));
        }

        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
}

Node *Editor::find_node(int input_id) {
    int output_id = -1;
    for (auto & link : links) {
        if (link.second == input_id) {
            output_id = link.first;
            break;
        }
    }
    if (output_id == -1) {
        return nullptr;
    }
    for (auto &node: nodes) {
        for (int i = 0; i < node->output_attr_ids.size(); ++i) {
            if (node->output_attr_ids[i] == output_id) {
                return node.get();
            }
        }
    }
    return nullptr;
}
