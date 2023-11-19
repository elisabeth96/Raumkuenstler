//
// Created by elisabeth on 19.11.23.
//
#pragma once

#include <vector>
#include <glm/vec3.hpp>

struct Editor;

class Node {
public:
    Node(Editor *editor, int num_inputs, int num_outputs);

    Editor *editor;
    int node_id;
    std::vector<int> input_attr_ids;
    std::vector<int> output_attr_ids;

    virtual void draw() = 0;

    virtual glm::dvec3 evaluate(glm::dvec3 p) = 0;
};

class OutputNode : public Node {
public:
    OutputNode(Editor *editor) : Node(editor, 1, 0) {}

    void draw() override;

    glm::dvec3 evaluate(glm::dvec3 p) override;
};

class SphereNode : public Node {
public:
    SphereNode(Editor *editor) : Node(editor, 2, 1) {}

    void draw() override;
    glm::dvec3 evaluate(glm::dvec3 p) override;
};

class ScalarNode : public Node {
public:
    float value = 0;

    ScalarNode(Editor *editor) : Node(editor, 0, 1) {}

    void draw() override;
    glm::dvec3 evaluate(glm::dvec3 p) override;
};

class PointNode : public Node {
public:
    glm::vec3 value = {0, 0, 0};

    PointNode(Editor *editor) : Node(editor, 0, 1) {}

    void draw() override;
    glm::dvec3 evaluate(glm::dvec3 p) override;
};
