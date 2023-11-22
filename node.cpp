//
// Created by elisabeth on 19.11.23.
//

#include "node.h"

#include "imnodes.h"
#include "editor.h"
#include "compiler.h"

#include <glm/glm.hpp>
#include <map>

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

std::vector<int> OutputNode::evaluate(std::vector<Instruction>& instructions, int& current_register, std::map<int, double>& constants) {
    Node *node = m_editor->find_node(m_node_id, 0);
    return node->evaluate(instructions, current_register, constants);
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

std::vector<int> SphereNode::evaluate(std::vector<Instruction>& instructions, int& current_register, std::map<int, double>& constants) {
    Node *node_center = m_editor->find_node(m_node_id, 0);
    Node *node_radius = m_editor->find_node(m_node_id, 1);
    std::vector<int> center = node_center->evaluate(instructions, current_register, constants);
    std::vector<int> radius = node_radius->evaluate(instructions, current_register, constants);
    glm::ivec3 res1 = generate_sub(instructions, current_register, {0,1,2}, {center[0], center[1], center[2]});
    int res2 = generate_length(instructions, current_register, res1);
    return {generate_sub(instructions, current_register, res2, radius[0])};
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

std::vector<int> TorusNode::evaluate(std::vector<Instruction>& instructions, int& current_register, std::map<int, double>& constants) {
    Node *node_radius1 = m_editor->find_node(m_node_id, 0);
    Node *node_radius2 = m_editor->find_node(m_node_id, 1);
    std::vector<int> r1 = node_radius1->evaluate(instructions, current_register, constants);
    std::vector<int> r2 = node_radius2->evaluate(instructions, current_register, constants);
    Instruction i0= {0, 0, current_register++, Operation::Mul};
    Instruction i1={2, 2, current_register++, Operation::Mul};
    Instruction i2={i0.output, i1.output, current_register++, Operation::Add};
    Instruction i3 = {i2.output, -1, current_register++, Operation::Sqrt};
    Instruction i4 = {i3.output, r1[0], current_register++, Operation::Sub};
    Instruction i5 = {i4.output, i4.output, current_register++, Operation::Mul};
    Instruction i6 = {1, 1, current_register++, Operation::Mul};
    Instruction i7 = {i5.output, i6.output, current_register++, Operation::Add};
    Instruction i8 = {i7.output, -1, current_register++, Operation::Sqrt};
    Instruction i9 = {i8.output, r2[0], current_register++, Operation::Sub};
    instructions.insert(instructions.end(), {i0, i1, i2, i3, i4, i5, i6, i7, i8, i9});
    return {i9.output};
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

std::vector<int> BoxNode::evaluate(std::vector<Instruction>& instructions, int& current_register, std::map<int, double>& constants) {
    Node *node_input = m_editor->find_node(m_node_id, 0);
    std::vector<int> input = node_input->evaluate(instructions, current_register, constants);
    Instruction i0= {0, -1, current_register++, Operation::Abs};
    Instruction i1={1, -1, current_register++, Operation::Abs};
    Instruction i2={2, -1, current_register++, Operation::Abs};
    Instruction qx = {i0.output, input[0], current_register++, Operation::Sub};
    Instruction qy = {i1.output, input[1], current_register++, Operation::Sub};
    Instruction qz = {i2.output, input[2], current_register++, Operation::Sub};
    Instruction i6 = {qy.output, qz.output, current_register++, Operation::Max};
    Instruction i7 = {qx.output, i6.output, current_register++, Operation::Max};
    int zero = current_register++;
    constants[zero] = 0;
    Instruction i8 = {i7.output, zero, current_register++, Operation::Min};
    Instruction i9 = {qx.output, zero, current_register++, Operation::Max};
    Instruction i10 = {qy.output, zero, current_register++, Operation::Max};
    Instruction i11 = {qz.output, zero, current_register++, Operation::Max};
    instructions.insert(instructions.end(), {i0, i1, i2, qx, qy, qz, i6, i7, i8, i9, i10, i11});
    int res = generate_length(instructions, current_register, {i9.output, i10.output, i11.output});
    Instruction i12 = {res, i8.output, current_register++, Operation::Add};
    instructions.push_back(i12);
    return {i12.output};
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

std::vector<int> ScalarNode::evaluate(std::vector<Instruction>& instructions, int& current_register, std::map<int, double>& constants) {
    constants[current_register] = value;
    return {current_register++};
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

std::vector<int> PointNode::evaluate(std::vector<Instruction>& instructions, int& current_register, std::map<int, double>& constants) {
    std::vector<int> num = {current_register, current_register + 1, current_register + 2};
    constants[current_register++] = value.x;
    constants[current_register++] = value.y;
    constants[current_register++] = value.z;
    return num;
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

std::vector<int> UnionNode::evaluate(std::vector<Instruction>& instructions, int& current_register, std::map<int, double>& constants) {
    Node *node_input1 = m_editor->find_node(m_node_id, 0);
    Node *node_input2 = m_editor->find_node(m_node_id, 1);
    std::vector<int> v1 = node_input1->evaluate(instructions, current_register, constants);
    std::vector<int> v2 = node_input2->evaluate(instructions, current_register, constants);
    Instruction i0= {v1[0], v2[0], current_register++, Operation::Min};
    instructions.push_back(i0);
    return {i0.output};
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

std::vector<int> SmoothUnionNode::evaluate(std::vector<Instruction>& instructions, int& current_register, std::map<int, double>& constants) {
    Node *node_input1 = m_editor->find_node(m_node_id, 0);
    Node *node_input2 = m_editor->find_node(m_node_id, 1);
    Node *node_input3 = m_editor->find_node(m_node_id, 2);
    std::vector<int> v1 = node_input1->evaluate(instructions, current_register, constants);
    std::vector<int> v2 = node_input2->evaluate(instructions, current_register, constants);
    std::vector<int> r = node_input3->evaluate(instructions, current_register, constants);
    Instruction i0= {r[0], v1[0], current_register++, Operation::Sub};
    Instruction i1= {r[0], v2[0], current_register++, Operation::Sub};
    int zero = current_register++;
    constants[zero] = 0;
    Instruction i2= {i0.output, zero, current_register++, Operation::Max};
    Instruction i3= {i1.output, zero, current_register++, Operation::Max};
    Instruction i4= {v1[0], v2[0], current_register++, Operation::Min};
    Instruction i5= {i4.output, r[0], current_register++, Operation::Max};
    instructions.insert(instructions.end(), {i0, i1, i2, i3, i4, i5});
    int res = generate_length(instructions, current_register, {i2.output, i3.output});
    Instruction i6 = {i5.output, res, current_register++, Operation::Sub};
    instructions.push_back(i6);
    return {i6.output};
}
