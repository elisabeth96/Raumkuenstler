//
// Created by elisabeth on 19.11.23.
//
#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <map>

#include "compiler.h"

enum class Type {
    Point,
    Scalar,
    None
};

struct Editor;

class Node {
public:
    Node(Editor *editor, int node_id, int num_inputs);

    Editor *m_editor;
    // The node id corresponds to the index in the editor's m_nodes vector
    int m_node_id;
    int m_num_inputs;
    Type m_output_type;

    // Draw the node
    virtual void draw() = 0;

    // Returns the register id(s) of the output of the node
    virtual std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) = 0;
};

class OutputNode : public Node {
public:
    constexpr static Type InputType[] = {Type::Scalar};

    OutputNode(Editor *editor, int node_id) : Node(editor, node_id, 1) {
        m_output_type = Type::None;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class SphereNode : public Node {
public:
    constexpr static Type InputType[] = {Type::Point, Type::Scalar};

    float m_radius = 0.2;
    glm::vec3 m_center = {0, 0, 0};

    SphereNode(Editor *editor, int node_id) : Node(editor, node_id, 2) {
        m_output_type = Type::Scalar;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class TorusNode : public Node {
public:
    constexpr static Type InputType[] = { Type::Scalar, Type::Scalar, Type::Point};
    float m_major_r = 0.3;
    float m_minor_r = 0.2;
    glm::vec3 m_center = {0, 0, 0};

    TorusNode(Editor *editor, int node_id) : Node(editor, node_id, 3) {
        m_output_type = Type::Scalar;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class BoxNode : public Node {
public:
    constexpr static Type InputType[] = {Type::Point, Type::Point};
    glm::vec3 m_center = {0, 0, 0};
    glm::vec3 m_size = {0.2, 0.3, 0.4};

    BoxNode(Editor *editor, int node_id) : Node(editor, node_id, 2) {
        m_output_type = Type::Scalar;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class CylinderNode : public Node {
public:
    constexpr static Type InputType[] = {Type::Scalar, Type::Scalar, Type::Point};
    glm::vec3 m_center = {0, 0, 0};
    float m_height = 0.4;
    float m_radius = 0.2;

    CylinderNode(Editor *editor, int node_id) : Node(editor, node_id, 3) {
        m_output_type = Type::Scalar;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class ScalarNode : public Node {
public:
    constexpr static Type InputType[] = {};
    float value = 0;

    ScalarNode(Editor *editor, int node_id) : Node(editor, node_id, 0) {
        m_output_type = Type::Scalar;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class PointNode : public Node {
public:
    constexpr static Type InputType[] = {Type::Scalar, Type::Scalar, Type::Scalar};
    glm::vec3 value = {0, 0, 0};

    PointNode(Editor *editor, int node_id) : Node(editor, node_id, 3) {
        m_output_type = Type::Point;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class TimeNode : public Node {
public:
    constexpr static Type InputType[] = {};
    TimeNode(Editor *editor, int node_id) : Node(editor, node_id, 0) {
        m_output_type = Type::Scalar;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class UnionNode : public Node {
public:
    constexpr static Type InputType[] = {Type::Scalar, Type::Scalar};
    UnionNode(Editor *editor, int node_id) : Node(editor, node_id, 2) {
        m_output_type = Type::Scalar;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class SmoothUnionNode : public Node {
public:
    constexpr static Type InputType[] = {Type::Scalar, Type::Scalar, Type::Scalar};
    float m_rounding = 0.05;

    SmoothUnionNode(Editor *editor, int node_id) : Node(editor, node_id, 3) {
        m_output_type = Type::Scalar;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class UnaryOpNode : public Node {
public:
    constexpr static Type InputType[] = {Type::Scalar};
    Operation m_op;

    UnaryOpNode(Editor *editor, int node_id, Operation op) : Node(editor, node_id, 1) {
        m_output_type = Type::Scalar;
        m_op = op;
    }

    void draw() override;

    std::vector<int>
    generate_instructions(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

