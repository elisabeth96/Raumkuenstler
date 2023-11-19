//
// Created by elisabeth on 19.11.23.
//

#pragma once

#include <vector>
#include <memory>

#include "node.h"

class Editor {
public:
    // contains nodes and connections between them
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<std::pair<int, int>> links;
    OutputNode output = OutputNode(this);
    int current_id = 2;

    void draw();
    void handle_links();
    void draw_node_dropdown();
    Node* find_node(int id);
};

