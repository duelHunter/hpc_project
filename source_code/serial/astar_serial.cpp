#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include "../common/node.h"
#include "../common/grid.h"
#include "../common/utils.h"

using namespace std;

/**
 * Serial A* Pathfinding Implementation
 */
class AStarSerial {
private:
    Grid& grid;
    int startX, startY;
    int goalX, goalY;
    
    // Custom comparator for priority queue (min-heap)
    struct CompareNode {
        bool operator()(const Node* a, const Node* b) const {
            return a->f > b->f;
        }
    };
    
public:
    AStarSerial(Grid& g, int sx, int sy, int gx, int gy)
        : grid(g), startX(sx), startY(sy), goalX(gx), goalY(gy) {}
    
    /**
     * Find path using A* algorithm
     */
    vector<pair<int, int>> findPath() {
        // Priority queue for open list
        priority_queue<Node*, vector<Node*>, CompareNode> openList;
        
        // Closed list to track visited nodes
        vector<vector<bool>> closedList(grid.getHeight(), 
                                       vector<bool>(grid.getWidth(), false));
        
        // Node storage (for memory management)
        vector<Node*> allNodes;
        
        // Create start node
        Node* startNode = new Node(startX, startY, 0, 
                                   manhattanDistance(startX, startY, goalX, goalY));
        allNodes.push_back(startNode);
        openList.push(startNode);
        
        Node* goalNode = nullptr;
        int nodesExpanded = 0;
        
        // Main A* loop
        while (!openList.empty()) {
            // Get node with lowest f cost
            Node* current = openList.top();
            openList.pop();
            
            // Skip if already visited
            if (closedList[current->x][current->y]) {
                continue;
            }
            
            // Mark as visited
            closedList[current->x][current->y] = true;
            nodesExpanded++;
            
            // Check if goal reached
            if (current->x == goalX && current->y == goalY) {
                goalNode = current;
                break;
            }
            
            // Explore neighbors (4 directions)
            for (int i = 0; i < NUM_DIRECTIONS; ++i) {
                int newX = current->x + DX[i];
                int newY = current->y + DY[i];
                
                // Check if neighbor is valid
                if (!grid.isValid(newX, newY)) {
                    continue;
                }
                
                // Skip if already visited
                if (closedList[newX][newY]) {
                    continue;
                }
                
                // Calculate costs
                int newG = current->g + 1;  // Cost to move to neighbor
                int newH = manhattanDistance(newX, newY, goalX, goalY);
                
                // Create neighbor node
                Node* neighbor = new Node(newX, newY, newG, newH, current);
                allNodes.push_back(neighbor);
                openList.push(neighbor);
            }
        }
        
        // Reconstruct path
        vector<pair<int, int>> path;
        if (goalNode != nullptr) {
            Node* current = goalNode;
            while (current != nullptr) {
                path.push_back({current->x, current->y});
                current = current->parent;
            }
            reverse(path.begin(), path.end());
        }
        
        // Cleanup
        for (Node* node : allNodes) {
            delete node;
        }
        
        cout << "Nodes expanded: " << nodesExpanded << endl;
        
        return path;
    }
};

int main(int argc, char* argv[]) {
    // Default parameters
    int gridWidth = 1000;
    int gridHeight = 1000;
    double obstacleDensity = 0.3;
    unsigned int seed = 42;
    
    // Parse command line arguments
    if (argc >= 3) {
        gridWidth = atoi(argv[1]);
        gridHeight = atoi(argv[2]);
    }
    if (argc >= 4) {
        obstacleDensity = atof(argv[3]);
    }
    if (argc >= 5) {
        seed = atoi(argv[4]);
    }
    
    cout << "========================================" << endl;
    cout << "Serial A* Pathfinding" << endl;
    cout << "========================================" << endl;
    cout << "Grid size: " << gridWidth << "x" << gridHeight << endl;
    cout << "Obstacle density: " << (obstacleDensity * 100) << "%" << endl;
    cout << "========================================" << endl;
    
    // Create grid
    Grid grid(gridWidth, gridHeight);
    grid.generateObstacles(obstacleDensity, seed);
    
    // Set start and goal positions
    int startX = 0;
    int startY = 0;
    int goalX = gridHeight - 1;
    int goalY = gridWidth - 1;
    
    // Ensure start and goal are not obstacles
    grid.clearCell(startX, startY);
    grid.clearCell(goalX, goalY);
    
    cout << "Start: (" << startX << ", " << startY << ")" << endl;
    cout << "Goal: (" << goalX << ", " << goalY << ")" << endl;
    cout << "========================================" << endl;
    
    // Create A* solver
    AStarSerial astar(grid, startX, startY, goalX, goalY);
    
    // Start timer
    Timer timer;
    timer.start();
    
    // Find path
    vector<pair<int, int>> path = astar.findPath();
    
    // Stop timer
    double executionTime = timer.stop();
    
    // Print results
    cout << "========================================" << endl;
    if (path.empty()) {
        cout << "No path found!" << endl;
    } else {
        cout << "Path found!" << endl;
        cout << "Path length: " << path.size() << " steps" << endl;
        printPath(path);
    }
    cout << "Execution time: " << executionTime << " ms" << endl;
    cout << "========================================" << endl;
    
    // Save results
    saveResults("../results/performance_logs.txt", "serial",
                gridWidth, gridHeight, 1, 1, executionTime, path.size());
    
    // Export visualization (for small grids)
    if (gridWidth <= 100 && gridHeight <= 100) {
        exportGridWithPath("../results/serial_path.txt", grid.getData(), 
                          path, startX, startY, goalX, goalY);
    }
    
    return 0;
}
