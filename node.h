//
// Created by elisabeth on 19.11.23.
//
#pragma once

#include <vector>
#include <glm/vec3.hpp>

struct Editor;

class Node {
public:
    Node(Editor *editor, int node_id, int num_inputs);

    Editor *m_editor;
    int m_node_id;
    int m_num_inputs;

    virtual void draw() = 0;

    virtual glm::dvec3 evaluate(glm::dvec3 p) = 0;
};

class OutputNode : public Node {
public:
    OutputNode(Editor *editor, int node_id) : Node(editor, node_id, 1) {}

    void draw() override;

    glm::dvec3 evaluate(glm::dvec3 p) override;
};

class SphereNode : public Node {
public:
    SphereNode(Editor *editor, int node_id) : Node(editor, node_id, 2) {}

    void draw() override;
    glm::dvec3 evaluate(glm::dvec3 p) override;
};

class TorusNode : public Node {
public:
    TorusNode(Editor *editor, int node_id) : Node(editor, node_id, 2) {}

    void draw() override;
    glm::dvec3 evaluate(glm::dvec3 p) override;
};

class BoxNode : public Node {
public:
    BoxNode(Editor *editor, int node_id) : Node(editor, node_id, 1) {}

    void draw() override;
    glm::dvec3 evaluate(glm::dvec3 p) override;
};

class ScalarNode : public Node {
public:
    float value = 0;

    ScalarNode(Editor *editor, int node_id) : Node(editor, node_id, 0) {}

    void draw() override;
    glm::dvec3 evaluate(glm::dvec3 p) override;
};

class PointNode : public Node {
public:
    glm::vec3 value = {0, 0, 0};

    PointNode(Editor *editor, int node_id) : Node(editor, node_id, 0) {}

    void draw() override;
    glm::dvec3 evaluate(glm::dvec3 p) override;
};
