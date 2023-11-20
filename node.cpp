//
// Created by elisabeth on 19.11.23.
//

#include "node.h"

#include "imnodes.h"
#include "editor.h"

#include <glm/glm.hpp>

Node::Node(Editor *editor, int node_id, int num_inputs) {
    this->m_editor = editor;
    this->m_node_id = node_id;
    this->m_num_inputs = num_inputs;
}

void OutputNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Output");
    ImNodes::EndNodeTitleBar();
    ImGui::Dummy(ImVec2(120.0f, 0.0f));

    assert(m_num_inputs == 1);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    ImNodes::EndInputAttribute();

    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 OutputNode::evaluate(glm::dvec3 p) {
    Node *node = m_editor->find_node(m_node_id, 0);
    return node->evaluate(p);
}

void SphereNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Sphere");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 2);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    ImGui::Text("center");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 1));
    ImGui::Text("radius");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 SphereNode::evaluate(glm::dvec3 p) {
    Node *node_center = m_editor->find_node(m_node_id, 0);
    Node *node_radius = m_editor->find_node(m_node_id, 1);
    return {glm::distance(p, node_center->evaluate(p)) - node_radius->evaluate(p).x, 0, 0};
}

void TorusNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Torus");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 2);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    ImGui::Text("major radius");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 1));
    ImGui::Text("minor radius");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 TorusNode::evaluate(glm::dvec3 p) {
    Node *node_radius1 = m_editor->find_node(m_node_id, 0);
    Node *node_radius2 = m_editor->find_node(m_node_id, 1);
    double r1 = node_radius1->evaluate(p).x;
    double r2 = node_radius2->evaluate(p).x;
    double x = sqrt(p.x * p.x + p.z * p.z) - r1;
    return {sqrt(x * x + p.y * p.y) - r2, 0, 0};
}

void BoxNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Box");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 1);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    ImGui::Text("Width, Height, Depth");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 BoxNode::evaluate(glm::dvec3 p) {
    Node *node_input = m_editor->find_node(m_node_id, 0);
    glm::dvec3 q = abs(p) - node_input->evaluate(p);
    return {glm::length(max(q, 0.0)) + std::min(std::max(q.x, std::max(q.y, q.z)), 0.0), 0, 0};
}

void ScalarNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Scalar");
    ImNodes::EndNodeTitleBar();
//ImGui::Dummy(ImVec2(80.0f, 45.0f));

    assert(m_num_inputs == 0);
    if (ImGui::InputFloat("value", &value, 0.1f, 1.0f, "%.3f")){
        m_editor->m_remesh = true;
    }

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 ScalarNode::evaluate(glm::dvec3 p) {
    return {value, 0, 0};
}

void PointNode::draw() {
    ImGui::PushItemWidth(240);
    ImNodes::BeginNode(m_node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Point");
    ImNodes::EndNodeTitleBar();
//ImGui::Dummy(ImVec2(80.0f, 45.0f));

    assert(m_num_inputs == 0);
    if (ImGui::InputFloat3("##", &value[0], "%.3f")){
        m_editor->m_remesh = true;
    }

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 PointNode::evaluate(glm::dvec3 p) {
    return value;
}

void UnionNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Union");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 2);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    ImGui::Text("Implicit 1");
    ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 1));
    ImGui::Text("Implicit 2");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 UnionNode::evaluate(glm::dvec3 p) {
    Node *node_input1 = m_editor->find_node(m_node_id, 0);
    Node *node_input2 = m_editor->find_node(m_node_id, 1);
    double v1 = node_input1->evaluate(p).x;
    double v2 = node_input2->evaluate(p).x;
    return {std::min(v1, v2), 0, 0};
}

void SmoothUnionNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Union");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 2);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    ImGui::Text("Implicit 1");
    ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 1));
    ImGui::Text("Implicit 2");
    ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 2));
    ImGui::Text("Rounding");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

glm::dvec3 SmoothUnionNode::evaluate(glm::dvec3 p) {
    Node *node_input1 = m_editor->find_node(m_node_id, 0);
    Node *node_input2 = m_editor->find_node(m_node_id, 1);
    Node *node_input3 = m_editor->find_node(m_node_id, 2);
    double v1 = node_input1->evaluate(p).x;
    double v2 = node_input2->evaluate(p).x;
    double r = node_input3->evaluate(p).x;
    glm::dvec2 u = max(glm::dvec2(r - v1,r - v2), glm::dvec2(0));
    return {std::max(r, std::min (v1, v2)) - length(u), 0, 0};
}
