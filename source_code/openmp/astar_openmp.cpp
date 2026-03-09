#include <iostream>     // for input/output
#include <vector>       // for dynamic arrays
#include <queue>        // for priority queue
#include <tuple>        // for storing multiple values together
#include <algorithm>    // for reverse()
#include <climits>      // for INT_MAX
#include <omp.h>        // OpenMP library for parallel programming
#include "../common/grid.h"   // custom grid class
#include "../common/utils.h"  // timer, printing, saving results

using namespace std;

// Class for OpenMP-based parallel A* pathfinding
class AStarOpenMP {
    Grid& grid;         // reference to the grid
    int sx, sy;         // start position (x, y)
    int gx, gy;         // goal position (x, y)
    int H, W;           // grid height and width
    int numThreads;     // number of OpenMP threads to use

    // Heuristic function: Manhattan distance
    // Estimates distance from current node to goal
    inline int h(int x, int y) const { 
        return abs(x - gx) + abs(y - gy); 
    }

public:
    // Constructor: initialize all required values
    AStarOpenMP(Grid& g, int sx, int sy, int gx, int gy, int threads)
        : grid(g), sx(sx), sy(sy), gx(gx), gy(gy),
          H(g.getHeight()), W(g.getWidth()), numThreads(threads) {}

    // Main function that finds the path
    vector<pair<int,int>> findPath() {
        // gScore stores shortest known distance from start to each node
        vector<int> gScore(H * W, INT_MAX);

        // parent stores previous node for path reconstruction
        vector<int> parent(H * W, -1);

        // closed marks whether a node is already fully processed
        vector<bool> closed(H * W, false);

        // Priority queue for open list
        // Stores pair (f_cost, node_index)
        using PQ = pair<int,int>;
        priority_queue<PQ, vector<PQ>, greater<PQ>> openList;

        // Convert start cell (sx, sy) into 1D index
        int startIdx = sx * W + sy;

        // Start node cost is 0
        gScore[startIdx] = 0;

        // -2 means special marker for start node
        parent[startIdx] = -2;

        // Push start node into open list with heuristic cost
        openList.push({h(sx, sy), startIdx});

        // Batch size = number of nodes taken together for parallel expansion
        // More threads -> bigger batch
        const int BATCH = numThreads * 4;

        int expanded = 0;            // count how many nodes were expanded
        bool found = false;          // becomes true when goal is reached
        int goalIdx = gx * W + gy;   // 1D index of goal node

        // Main loop: continue until openList is empty or goal found
        while (!openList.empty() && !found) {

            // -----------------------------
            // Step 1: Collect a batch of best nodes
            // -----------------------------
            vector<int> batch;
            batch.reserve(BATCH);

            while (!openList.empty() && (int)batch.size() < BATCH) {
                auto [f, idx] = openList.top(); 
                openList.pop();

                // Skip if already closed
                if (closed[idx]) continue;

                // Mark node as processed
                closed[idx] = true;
                ++expanded;

                // Add node to batch
                batch.push_back(idx);

                // If goal found, stop
                if (idx == goalIdx) { 
                    found = true; 
                    break; 
                }
            }

            if (found) break;

            // -----------------------------
            // Step 2: Expand batch in parallel
            // -----------------------------
            #pragma omp parallel num_threads(numThreads)
            {
                // Each thread stores generated neighbours locally first
                // tuple = (f_cost, neighbour_index, new_g, parent_index)
                vector<tuple<int,int,int,int>> localGen;
                localGen.reserve(BATCH * 4);

                // Divide batch nodes among threads dynamically
                #pragma omp for schedule(dynamic,1)
                for (int b = 0; b < (int)batch.size(); ++b) {
                    int idx = batch[b];

                    // Convert 1D index back to 2D coordinates
                    int cx = idx / W;
                    int cy = idx % W;

                    // Current path cost
                    int cg = gScore[idx];

                    // Check all possible movement directions
                    for (int i = 0; i < NUM_DIRECTIONS; ++i) {
                        int nx = cx + DX[i];
                        int ny = cy + DY[i];

                        // Skip invalid or blocked cells
                        if (!grid.isValid(nx, ny)) continue;

                        int nIdx = nx * W + ny;

                        // Skip if already processed
                        if (closed[nIdx]) continue;

                        // New cost from start to neighbour
                        int ng = cg + 1;

                        // Optimistic check before entering critical section
                        if (ng < gScore[nIdx]) {
                            // Store candidate neighbour in local buffer
                            localGen.emplace_back(ng + h(nx, ny), nIdx, ng, idx);
                        }
                    }
                }

                // -----------------------------
                // Step 3: Merge local results safely
                // -----------------------------
                #pragma omp critical
                {
                    for (auto& [f, nIdx, ng, pIdx] : localGen) {
                        // Recheck before updating shared data
                        if (!closed[nIdx] && ng < gScore[nIdx]) {
                            gScore[nIdx] = ng;      // update best cost
                            parent[nIdx] = pIdx;    // store parent
                            openList.push({f, nIdx}); // push into priority queue
                        }
                    }
                }
            } // end parallel section
        }

        cout << "Nodes expanded: " << expanded << endl;

        // -----------------------------
        // Step 4: Reconstruct the path
        // -----------------------------
        vector<pair<int,int>> path;

        if (found) {
            int idx = goalIdx;

            // Follow parent links from goal back to start
            while (idx != -2) {
                path.push_back({idx / W, idx % W});
                idx = parent[idx];
            }

            // Reverse to get path from start to goal
            reverse(path.begin(), path.end());
        }

        return path;
    }
};

int main(int argc, char* argv[]) {
    // Default values
    int W = 1000, H = 1000;         // grid size
    double density = 0.3;           // obstacle density
    unsigned int seed = 42;         // random seed
    int numThreads = omp_get_max_threads(); // default = max available threads

    // Read command-line arguments if provided
    if (argc >= 3) { 
        W = atoi(argv[1]); 
        H = atoi(argv[2]); 
    }
    if (argc >= 4) density = atof(argv[3]);
    if (argc >= 5) seed = atoi(argv[4]);
    if (argc >= 6) numThreads = atoi(argv[5]);

    // Set number of OpenMP threads
    omp_set_num_threads(numThreads);

    // Print configuration
    cout << "========================================" << endl;
    cout << "OpenMP Parallel A* Pathfinding" << endl;
    cout << "Grid size: " << W << "x" << H << endl;
    cout << "Obstacle density: " << (density * 100) << "%" << endl;
    cout << "Threads: " << numThreads << endl;
    cout << "========================================" << endl;

    // Create grid and generate random obstacles
    Grid grid(W, H);
    grid.generateObstacles(density, seed);

    // Define start and goal positions
    int startX = 0, startY = 0;
    int goalX = H - 1, goalY = W - 1;

    // Ensure start and goal cells are free
    grid.clearCell(startX, startY);
    grid.clearCell(goalX, goalY);

    cout << "Start: (" << startX << "," << startY << ") | Goal: (" 
         << goalX << "," << goalY << ")" << endl;

    // Create A* OpenMP object
    AStarOpenMP astar(grid, startX, startY, goalX, goalY, numThreads);

    // Start timer
    Timer timer; 
    timer.start();

    // Run pathfinding
    auto path = astar.findPath();

    // Stop timer
    double t = timer.stop();

    cout << "========================================" << endl;

    if (path.empty()) 
        cout << "No path found!" << endl;
    else {
        cout << "Path found! Length: " << path.size() << " steps" << endl;
        printPath(path);   // print resulting path
    }

    cout << "Execution time: " << t << " ms" << endl;
    cout << "Threads used: " << numThreads << endl;
    cout << "========================================" << endl;

    // Save performance data to file
    saveResults("../results/performance_logs.txt", "openmp", W, H, numThreads, 1, t, path.size());

    return 0;
}