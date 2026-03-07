#ifndef NODE_H
#define NODE_H

#include <memory>

/**
 * Node structure for A* pathfinding
 * Each node represents a cell in the grid
 */
struct Node {
    int x;              // X coordinate
    int y;              // Y coordinate
    int g;              // Cost from start to this node
    int h;              // Heuristic cost to goal
    int f;              // Total cost (g + h)
    Node* parent;       // Pointer to parent node for path reconstruction
    
    // Constructor
    Node(int x = 0, int y = 0, int g = 0, int h = 0, Node* parent = nullptr)
        : x(x), y(y), g(g), h(h), f(g + h), parent(parent) {}
    
    // Update costs
    void updateCosts(int newG, int newH) {
        g = newG;
        h = newH;
        f = g + h;
    }
    
    // Equality comparison
    bool operator==(const Node& other) const {
        return x == other.x && y == other.y;
    }
    
    // For priority queue (min-heap based on f cost)
    bool operator>(const Node& other) const {
        return f > other.f;
    }
    
    bool operator<(const Node& other) const {
        return f < other.f;
    }
};

// Hash function for Node (for unordered_set/map)
struct NodeHash {
    std::size_t operator()(const Node& node) const {
        return std::hash<int>()(node.x) ^ (std::hash<int>()(node.y) << 1);
    }
};

// Manhattan distance heuristic
inline int manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

#endif // NODE_H
