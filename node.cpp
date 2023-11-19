//
// Created by elisabeth on 19.11.23.
//

#include "node.h"

#include "imnodes.h"
#include "editor.h"

#include <glm/glm.hpp>

Node::Node(Editor *editor, int num_inputs, int num_outputs) {
    this->editor = editor;
    node_id = editor->current_id++;
    for (int i = 0; i < num_inputs; ++i) {
        input_attr_ids.push_back(editor->current_id++);
    }
    for (int i = 0; i < num_outputs; ++i) {
        output_attr_ids.push_back(editor->current_id++);
    }
}

void OutputNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Output");
    ImNodes::EndNodeTitleBar();
    ImGui::Dummy(ImVec2(120.0f, 0.0f));

    assert(input_attr_ids.size() == 1);
    ImNodes::BeginInputAttribute(input_attr_ids[0]);
    ImNodes::EndInputAttribute();

    assert(output_attr_ids.size() == 0);

    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 OutputNode::evaluate(glm::dvec3 p) {
    Node *node = editor->find_node(input_attr_ids[0]);
    return node->evaluate(p);
}

void SphereNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Sphere");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(input_attr_ids.size() == 2);
    ImNodes::BeginInputAttribute(input_attr_ids[0]);
    ImGui::Text("center");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(input_attr_ids[1]);
    ImGui::Text("radius");
    ImNodes::EndInputAttribute();

    assert(output_attr_ids.size() == 1);
    ImNodes::BeginOutputAttribute(output_attr_ids[0]);
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 SphereNode::evaluate(glm::dvec3 p) {
    Node *node_center = editor->find_node(input_attr_ids[0]);
    Node *node_radius = editor->find_node(input_attr_ids[1]);
    return {glm::distance(p, node_center->evaluate(p)) - node_radius->evaluate(p).x, 0, 0};
}

void ScalarNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Scalar");
    ImNodes::EndNodeTitleBar();
//ImGui::Dummy(ImVec2(80.0f, 45.0f));

    assert(input_attr_ids.size() == 0);
    ImGui::SliderFloat("value", &value, -10, 10);

    assert(output_attr_ids.size() == 1);
    ImNodes::BeginOutputAttribute(output_attr_ids[0]);
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 ScalarNode::evaluate(glm::dvec3 p) {
    return {value, 0, 0};
}

void PointNode::draw() {
    ImGui::PushItemWidth(240);
    ImNodes::BeginNode(node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Point");
    ImNodes::EndNodeTitleBar();
//ImGui::Dummy(ImVec2(80.0f, 45.0f));

    assert(input_attr_ids.size() == 0);
    ImGui::SliderFloat3("coordinate", &value[0], -10, 10);

    assert(output_attr_ids.size() == 1);
    ImNodes::BeginOutputAttribute(output_attr_ids[0]);
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 PointNode::evaluate(glm::dvec3 p) {
    return value;
}
