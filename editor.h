//
// Created by elisabeth on 19.11.23.
//

#pragma once

#include <vector>
#include <memory>

#include "node.h"

constexpr int INPUT_ATTRIBUTE_OFFSET = 10e6;
constexpr int OUTPUT_ATTRIBUTE_OFFSET = 20e6;
class Editor {
public:
    Editor();

    // contains nodes and connections between them
    std::vector<std::unique_ptr<Node>> m_nodes;

    // For each node, we store its inputs. Each input consists of the input node id (possibly -1 if
    // there is no input link) and the input attribute id.
    std::vector<std::vector<std::pair<int, int>>> m_inputs;
    int m_current_input_id = 0;
    bool m_remesh = true;

    void draw();
    void handle_links();
    void draw_node_dropdown();
    Node* find_node(int node_id, int input_id);

    template <class T>
    void add_node();
    int get_input_attribute_id(int node_id, int input_id);
    int get_output_attribute_id(int node_id);
};

