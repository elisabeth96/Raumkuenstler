//
// Created by elisabeth on 19.11.23.
//

#pragma once

#include <vector>
#include <memory>

#include "node.h"

constexpr int INPUT_ATTRIBUTE_OFFSET = 10e6;
constexpr int OUTPUT_ATTRIBUTE_OFFSET = 20e6;

struct InputSlot {
    int node_id;
    int attribute_id;
    Type type;
};

class Editor {
public:
    Editor();

    // Contains nodes
    std::vector<std::unique_ptr<Node>> m_nodes;

    // For each node, we store its inputs. Each input consists of the input node id (possibly -1 if
    // there is no input link), the input attribute id and the input type (scalar or point).
    std::vector<std::vector<InputSlot>> m_inputs;
    int m_current_input_id = 0;
    bool m_remesh = true;

    // Draw the nodes
    void draw();

    // Check if links were added (or changed) and update the m_inputs vector accordingly
    void handle_links();

    void draw_primitive_dropdown();

    void draw_operator_dropdown();

    void draw_math_dropdown();

    // Find a node in the m_nodes vector
    Node *find_node(int node_id, int input_id);

    // Add a node to the m_nodes vector
    template<class T, Operation op = Operation::None>
    void add_node();

    int get_input_attribute_id(int node_id, int input_id);

    int get_output_attribute_id(int node_id);
};

