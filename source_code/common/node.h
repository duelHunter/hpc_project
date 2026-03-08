#ifndef NODE_H
#define NODE_H

#include <cstdlib>

struct Node {
    int x, y;
    int g, h, f;
    Node* parent;

    Node(int x = 0, int y = 0, int g = 0, int h = 0, Node* parent = nullptr)
        : x(x), y(y), g(g), h(h), f(g + h), parent(parent) {}

    bool operator>(const Node& o) const { return f > o.f; }
    bool operator<(const Node& o) const { return f < o.f; }
    bool operator==(const Node& o) const { return x == o.x && y == o.y; }
};

inline int manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

#endif
