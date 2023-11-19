//
// Created by elisabeth on 19.11.23.
//

#include "editor.h"
#include "imnodes.h"

void Editor::draw() {
    ImNodes::BeginNodeEditor();
    output.draw();
    for (auto &node: nodes) {
        node->draw();
    }
    for (int i = 0; i < links.size(); ++i) {
        ImNodes::Link(i, links[i].first, links[i].second);
    }
    ImNodes::EndNodeEditor();
}
