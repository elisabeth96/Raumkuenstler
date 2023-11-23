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

std::vector<int>
OutputNode::evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) {
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
    //ImGui::Text("center");
    if (m_editor->find_node(m_node_id, 0)) {
        ImGui::Text("center");
    } else {
        if (ImGui::InputFloat3("center", &m_center.x, "%.3f")) {
            m_editor->m_remesh = true;
        }
    }
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 1));
    //ImGui::Text("radius");
    // if no input draw a slider, otherwise just draw the input name
    if (m_editor->find_node(m_node_id, 1)) {
        ImGui::Text("radius");
    } else {
        if (ImGui::InputFloat("radius", &m_radius, 0.1f, 1.0f, "%.2f")) {
            m_editor->m_remesh = true;
        }
    }

    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

std::vector<int>
SphereNode::evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) {
    Node *node_center = m_editor->find_node(m_node_id, 0);
    Node *node_radius = m_editor->find_node(m_node_id, 1);
    std::vector<int> center;
    if (node_center) {
        center = node_center->evaluate(instructions, current_register, constants);
    } else {
        auto cx = make_constant(constants, current_register, m_center.x);
        auto cy = make_constant(constants, current_register, m_center.y);
        auto cz = make_constant(constants, current_register, m_center.z);
        center = {cx, cy, cz};
    }
    std::vector<int> radius;
    if (node_radius) {
        radius = node_radius->evaluate(instructions, current_register, constants);
    } else {
        radius = {make_constant(constants, current_register, m_radius)};
    }
    glm::ivec3 res1 = generate_sub(instructions, current_register, {0, 1, 2}, {center[0], center[1], center[2]});
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
    assert(m_num_inputs == 3);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    //ImGui::Text("major radius");
    if (m_editor->find_node(m_node_id, 0)) {
        ImGui::Text("major radius");
    } else {
        if (ImGui::InputFloat("major radius", &m_major_r, 0.1f, 1.0f, "%.3f")) {
            m_editor->m_remesh = true;
        }
    }
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 1));
    //ImGui::Text("minor radius");
    if (m_editor->find_node(m_node_id, 1)) {
        ImGui::Text("minor radius");
    } else {
        if (ImGui::InputFloat("minor radius", &m_minor_r, 0.1f, 1.0f, "%.3f")) {
            m_editor->m_remesh = true;
        }
    }
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 2));
    //ImGui::Text("center");
    if (m_editor->find_node(m_node_id, 2)) {
        ImGui::Text("center");
    } else {
        if (ImGui::InputFloat3("center", &m_center.x, "%.2f")) {
            m_editor->m_remesh = true;
        }
    }
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

std::vector<int>
TorusNode::evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) {
    Node *node_radius1 = m_editor->find_node(m_node_id, 0);
    Node *node_radius2 = m_editor->find_node(m_node_id, 1);
    Node *node_center = m_editor->find_node(m_node_id, 2);
    std::vector<int> r1;
    if (node_radius1) {
        r1 = node_radius1->evaluate(instructions, current_register, constants);
    } else {
        r1 = {make_constant(constants, current_register, m_major_r)};
    }
    std::vector<int> r2;
    if (node_radius2) {
        r2 = node_radius2->evaluate(instructions, current_register, constants);
    } else {
        r2 = {make_constant(constants, current_register, m_minor_r)};
    }
    std::vector<int> c;
    if (node_center) {
        c = node_center->evaluate(instructions, current_register, constants);
    } else {
        auto cx = make_constant(constants, current_register, m_center.x);
        auto cy = make_constant(constants, current_register, m_center.y);
        auto cz = make_constant(constants, current_register, m_center.z);
        c = {cx, cy, cz};
    }
    glm::ivec3 res0 = generate_sub(instructions, current_register, {0, 1, 2}, {c[0], c[1], c[2]});
    int res1 = generate_length(instructions, current_register, {res0.x, res0.z});
    int res2 = generate_sub(instructions, current_register, res1, r1[0]);
    int res3 = generate_length(instructions, current_register, {res2, res0.y});
    return {generate_sub(instructions, current_register, res3, r2[0])};
}

void BoxNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Box");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 2);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    //ImGui::Text("width, height, depth");
    if (m_editor->find_node(m_node_id, 0)) {
        ImGui::Text("size");
    } else {
        if (ImGui::InputFloat3("size", &m_size.x, "%.2f")) {
            m_editor->m_remesh = true;
        }
    }
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 1));
    //ImGui::Text("center");
    if (m_editor->find_node(m_node_id, 1)) {
        ImGui::Text("center");
    } else {
        if (ImGui::InputFloat3("center", &m_center.x, "%.2f")) {
            m_editor->m_remesh = true;
        }
    }
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

std::vector<int>
BoxNode::evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) {
    Node *node_input = m_editor->find_node(m_node_id, 0);
    Node *node_center = m_editor->find_node(m_node_id, 1);
    std::vector<int> input;
    if (node_input) {
        input = node_input->evaluate(instructions, current_register, constants);
    } else {
        auto cx = make_constant(constants, current_register, m_size.x);
        auto cy = make_constant(constants, current_register, m_size.y);
        auto cz = make_constant(constants, current_register, m_size.z);
        input = {cx, cy, cz};
    }
    std::vector<int> center;
    if (node_center) {
        center = node_center->evaluate(instructions, current_register, constants);
    } else {
        auto cx = make_constant(constants, current_register, m_center.x);
        auto cy = make_constant(constants, current_register, m_center.y);
        auto cz = make_constant(constants, current_register, m_center.z);
        center = {cx, cy, cz};
    }
    glm::ivec3 res0 = generate_sub(instructions, current_register, {0, 1, 2}, {center[0], center[1], center[2]});
    glm::ivec3 res1 = generate_abs(instructions, current_register, res0);
    glm::ivec3 res2 = generate_sub(instructions, current_register, res1, {input[0], input[1], input[2]});
    int res3 = generate_max_element(instructions, current_register, res2);
    int zero = current_register++;
    constants[zero] = 0;
    int res4 = generate_min(instructions, current_register, res3, zero);
    glm::ivec3 res5 = generate_max(instructions, current_register, res2, {zero, zero, zero});
    int res6 = generate_length(instructions, current_register, res5);
    return {generate_add(instructions, current_register, res4, res6)};
}

void ScalarNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Scalar");
    ImNodes::EndNodeTitleBar();
//ImGui::Dummy(ImVec2(80.0f, 45.0f));

    assert(m_num_inputs == 0);
    if (ImGui::InputFloat("value", &value, 0.1f, 1.0f, "%.3f")) {
        m_editor->m_remesh = true;
    }

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

std::vector<int>
ScalarNode::evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) {
    constants[current_register] = value;
    return {current_register++};
}

void PointNode::draw() {
    ImGui::PushItemWidth(240);
    ImNodes::BeginNode(m_node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Point");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(180.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 3);

    auto draw_input_coordinate = [&](int i, const char* name) {
        ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, i));
        if (m_editor->find_node(m_node_id, i)) {
            ImGui::Text(name);
        } else {
            if (ImGui::InputFloat(name, &value[i], 0.1f, 1.0f, "%.2f")) {
                m_editor->m_remesh = true;
            }
        }
        ImNodes::EndInputAttribute();
    };

    draw_input_coordinate(0, "x");
    draw_input_coordinate(1, "y");
    draw_input_coordinate(2, "z");

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

std::vector<int>
PointNode::evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) {
    Node* node_x = m_editor->find_node(m_node_id, 0);
    Node* node_y = m_editor->find_node(m_node_id, 1);
    Node* node_z = m_editor->find_node(m_node_id, 2);

    std::vector<int> value(3);
    if(node_x) {
        value[0] = node_x->evaluate(instructions, current_register, constants)[0];
    }
    else {
        value[0] = make_constant(constants, current_register, this->value.x);
    }
    if(node_y) {
        value[1] = node_y->evaluate(instructions, current_register, constants)[0];
    }
    else {
        value[1] = make_constant(constants, current_register, this->value.y);
    }
    if(node_z) {
        value[2] = node_z->evaluate(instructions, current_register, constants)[0];
    }
    else {
        value[2] = make_constant(constants, current_register, this->value.z);
    }
    return value;
}

void TimeNode::draw() {
    ImGui::PushItemWidth(240);
    ImNodes::BeginNode(m_node_id);
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Time");
    ImNodes::EndNodeTitleBar();
//ImGui::Dummy(ImVec2(80.0f, 45.0f));

    assert(m_num_inputs == 0);
    m_editor->m_remesh = true;

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

std::vector<int>
TimeNode::evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) {
    return {make_constant(constants, current_register, ImGui::GetTime())};
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

std::vector<int>
UnionNode::evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) {
    Node *node_input1 = m_editor->find_node(m_node_id, 0);
    Node *node_input2 = m_editor->find_node(m_node_id, 1);
    std::vector<int> v1 = node_input1->evaluate(instructions, current_register, constants);
    std::vector<int> v2 = node_input2->evaluate(instructions, current_register, constants);
    return {generate_min(instructions, current_register, v1[0], v2[0])};
}

void SmoothUnionNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("Union");
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 3);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    ImGui::Text("Implicit 1");
    ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 1));
    ImGui::Text("Implicit 2");
    ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 2));
    //ImGui::Text("Rounding");
    if (m_editor->find_node(m_node_id, 2)) {
        ImGui::Text("rounding");
    } else {
        if (ImGui::InputFloat("rounding", &m_rounding, 0.1f, 1.0f, "%.3f")) {
            m_editor->m_remesh = true;
        }
    }
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

std::vector<int> SmoothUnionNode::evaluate(std::vector<Instruction> &instructions, int &current_register,
                                           std::map<int, double> &constants) {
    Node *node_input1 = m_editor->find_node(m_node_id, 0);
    Node *node_input2 = m_editor->find_node(m_node_id, 1);
    Node *node_input3 = m_editor->find_node(m_node_id, 2);
    std::vector<int> v1 = node_input1->evaluate(instructions, current_register, constants);
    std::vector<int> v2 = node_input2->evaluate(instructions, current_register, constants);
    std::vector<int> r;
    if (node_input3) {
        r = node_input3->evaluate(instructions, current_register, constants);
    } else {
        r = {make_constant(constants, current_register, m_rounding)};
    }
    glm::ivec2 res1 = generate_sub(instructions, current_register, {r[0], r[0]}, {v1[0], v2[0]});
    int zero = current_register++;
    constants[zero] = 0;
    glm::ivec2 res2 = generate_max(instructions, current_register, res1, {zero, zero});
    int res3 = generate_min(instructions, current_register, v1[0], v2[0]);
    int res4 = generate_max(instructions, current_register, res3, r[0]);
    int res5 = generate_length(instructions, current_register, res2);
    return {generate_sub(instructions, current_register, res4, res5)};
}

void UnaryOpNode::draw() {
    ImGui::PushItemWidth(120);
    ImNodes::BeginNode(m_node_id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(make_op_name(m_op));
    ImNodes::EndNodeTitleBar();

    ImGui::Dummy(ImVec2(120.0f, 0.0f)); // Adjust width here
    assert(m_num_inputs == 1);
    ImNodes::BeginInputAttribute(m_editor->get_input_attribute_id(m_node_id, 0));
    ImGui::Text("Input");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(m_editor->get_output_attribute_id(m_node_id));
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();
    ImGui::PopItemWidth();
}

std::vector<int> UnaryOpNode::evaluate(std::vector<Instruction> &instructions, int &current_register,
                                           std::map<int, double> &constants) {
    Node *node_input = m_editor->find_node(m_node_id, 0);
    std::vector<int> input = node_input->evaluate(instructions, current_register, constants);
    switch (m_op) {
        case Operation::Sqrt:
            return {generate_sqrt(instructions, current_register, input[0])};
        case Operation::Abs:
            return {generate_abs(instructions, current_register, input[0])};
        case Operation::Sin:
            return {generate_sin(instructions, current_register, input[0])};
        case Operation::Cos:
            return {generate_cos(instructions, current_register, input[0])};
        default:
            assert(false);
    }
    return {-1};
}
