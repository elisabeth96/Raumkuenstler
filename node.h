//
// Created by elisabeth on 19.11.23.
//
#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <map>

#include "compiler.h"

struct Editor;

class Node {
public:
    Node(Editor *editor, int node_id, int num_inputs);

    Editor *m_editor;
    int m_node_id;
    int m_num_inputs;

    virtual void draw() = 0;

    virtual std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) = 0;
};

class OutputNode : public Node {
public:
    OutputNode(Editor *editor, int node_id) : Node(editor, node_id, 1) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class SphereNode : public Node {
public:
    float m_radius = 0.2;
    glm::vec3 m_center = {0, 0, 0};

    SphereNode(Editor *editor, int node_id) : Node(editor, node_id, 2) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class TorusNode : public Node {
public:
    float m_major_r = 0.3;
    float m_minor_r = 0.2;
    glm::vec3 m_center = {0, 0, 0};

    TorusNode(Editor *editor, int node_id) : Node(editor, node_id, 3) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class BoxNode : public Node {
public:
    glm::vec3 m_center = {0, 0, 0};
    glm::vec3 m_size = {0.2, 0.3, 0.4};

    BoxNode(Editor *editor, int node_id) : Node(editor, node_id, 2) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class ScalarNode : public Node {
public:
    float value = 0;

    ScalarNode(Editor *editor, int node_id) : Node(editor, node_id, 0) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class PointNode : public Node {
public:
    glm::vec3 value = {0, 0, 0};

    PointNode(Editor *editor, int node_id) : Node(editor, node_id, 3) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class TimeNode : public Node {
public:
    TimeNode(Editor *editor, int node_id) : Node(editor, node_id, 0) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class UnionNode : public Node {
public:
    UnionNode(Editor *editor, int node_id) : Node(editor, node_id, 2) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class SmoothUnionNode : public Node {
public:
    float m_rounding = 0.05;

    SmoothUnionNode(Editor *editor, int node_id) : Node(editor, node_id, 3) {}

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

class UnaryOpNode : public Node {
public:
    Operation m_op;

    UnaryOpNode(Editor *editor, int node_id, Operation op) : Node(editor, node_id, 1) {
        m_op = op;
    }

    void draw() override;

    std::vector<int>
    evaluate(std::vector<Instruction> &instructions, int &current_register, std::map<int, double> &constants) override;
};

