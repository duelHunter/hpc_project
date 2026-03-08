#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include "../common/node.h"
#include "../common/grid.h"
#include "../common/utils.h"

using namespace std;

class AStarSerial {
private:
    Grid& grid;
    int startX, startY, goalX, goalY;

    struct CompareNode {
        bool operator()(const Node* a, const Node* b) const { return a->f > b->f; }
    };

public:
    AStarSerial(Grid& g, int sx, int sy, int gx, int gy)
        : grid(g), startX(sx), startY(sy), goalX(gx), goalY(gy) {}

    vector<pair<int,int>> findPath() {
        priority_queue<Node*, vector<Node*>, CompareNode> openList;
        vector<vector<bool>> closed(grid.getHeight(), vector<bool>(grid.getWidth(), false));
        vector<Node*> allNodes;

        Node* startNode = new Node(startX, startY, 0,
                                   manhattanDistance(startX, startY, goalX, goalY));
        allNodes.push_back(startNode);
        openList.push(startNode);

        Node* goalNode = nullptr;
        int expanded = 0;

        while (!openList.empty()) {
            Node* cur = openList.top(); openList.pop();
            if (closed[cur->x][cur->y]) continue;
            closed[cur->x][cur->y] = true;
            ++expanded;

            if (cur->x == goalX && cur->y == goalY) { goalNode = cur; break; }

            for (int i = 0; i < NUM_DIRECTIONS; ++i) {
                int nx = cur->x + DX[i], ny = cur->y + DY[i];
                if (!grid.isValid(nx, ny) || closed[nx][ny]) continue;
                int ng = cur->g + 1;
                int nh = manhattanDistance(nx, ny, goalX, goalY);
                Node* nb = new Node(nx, ny, ng, nh, cur);
                allNodes.push_back(nb);
                openList.push(nb);
            }
        }

        vector<pair<int,int>> path;
        if (goalNode) {
            for (Node* c = goalNode; c; c = c->parent)
                path.push_back({c->x, c->y});
            reverse(path.begin(), path.end());
        }
        for (Node* n : allNodes) delete n;
        cout << "Nodes expanded: " << expanded << endl;
        return path;
    }
};

int main(int argc, char* argv[]) {
    int W = 1000, H = 1000;
    double density = 0.3;
    unsigned int seed = 42;

    if (argc >= 3) { W = atoi(argv[1]); H = atoi(argv[2]); }
    if (argc >= 4) density = atof(argv[3]);
    if (argc >= 5) seed = atoi(argv[4]);

    cout << "========================================" << endl;
    cout << "Serial A* Pathfinding" << endl;
    cout << "Grid size: " << W << "x" << H << endl;
    cout << "Obstacle density: " << (density * 100) << "%" << endl;
    cout << "========================================" << endl;

    Grid grid(W, H);
    grid.generateObstacles(density, seed);

    int sx = 0, sy = 0, gx = H - 1, gy = W - 1;
    grid.clearCell(sx, sy);
    grid.clearCell(gx, gy);
    cout << "Start: (" << sx << "," << sy << ") | Goal: (" << gx << "," << gy << ")" << endl;

    AStarSerial astar(grid, sx, sy, gx, gy);
    Timer timer;
    timer.start();
    auto path = astar.findPath();
    double t = timer.stop();

    cout << "========================================" << endl;
    if (path.empty()) cout << "No path found!" << endl;
    else {
        cout << "Path found! Length: " << path.size() << " steps" << endl;
        printPath(path);
    }
    cout << "Execution time: " << t << " ms" << endl;
    cout << "========================================" << endl;

    saveResults("../results/performance_logs.txt", "serial", W, H, 1, 1, t, path.size());

    if (W <= 100 && H <= 100)
        exportGridWithPath("../results/serial_path.txt", grid.getData(), path, sx, sy, gx, gy);

    return 0;
}
