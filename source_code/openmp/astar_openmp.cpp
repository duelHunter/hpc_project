
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
#include <atomic>
#include <omp.h>
#include "../common/grid.h"
#include "../common/utils.h"

using namespace std;

/*
 * Parallel A* using OpenMP - same algorithm as serial A*
 *
 * The serial A* pops one node per iteration. This version batches
 * all nodes sharing the minimum f-value and expands them in parallel.
 *
 * Steps per iteration:
 *   1. [Serial]   Extract all nodes with f == f_min from openList into batch
 *   2. [Parallel] Each thread expands its slice of batch, writing neighbors
 *                 into thread-local buffers (no lock on openList)
 *   3. [Serial]   Merge thread-local buffers into openList
 *
 * Data structures identical to serial A*:
 *   - priority_queue openList ordered by f = g + h
 *   - gScore[] - best known g-cost (atomic<int> for safe parallel writes)
 *   - parent[] - for path reconstruction
 *   - closed[] - atomic<bool>, prevents re-expanding a settled node
 */

static inline bool atomicMin(atomic<int>& target, int value) {
    int old = target.load(memory_order_relaxed);
    while (value < old)
        if (target.compare_exchange_weak(old, value, memory_order_relaxed))
            return true;
    return false;
}

class AStarOpenMP {
    Grid& grid;
    int sx, sy, gx, gy, H, W, T;
    inline int h(int x, int y) const { return abs(x-gx)+abs(y-gy); }
public:
    AStarOpenMP(Grid& g, int sx, int sy, int gx, int gy, int threads)
        : grid(g), sx(sx), sy(sy), gx(gx), gy(gy),
          H(g.getHeight()), W(g.getWidth()), T(threads) {}

    vector<pair<int,int>> findPath() {
        const int N = H * W;

        vector<atomic<int>>  gScore(N);
        for (auto& g : gScore) g.store(INT_MAX, memory_order_relaxed);

        vector<int>          parent(N, -1);

        vector<atomic<bool>> closed(N);
        for (auto& c : closed) c.store(false, memory_order_relaxed);

        using Entry = pair<int,int>;
        priority_queue<Entry, vector<Entry>, greater<Entry>> openList;

        int startIdx = sx*W + sy;
        int goalIdx  = gx*W + gy;
        gScore[startIdx].store(0, memory_order_relaxed);
        parent[startIdx] = -2;
        openList.push({h(sx,sy), startIdx});

        struct Discovery { int f, nIdx, parIdx, ng; };
        vector<vector<Discovery>> localBuf(T);
        for (auto& v : localBuf) v.reserve(256);

        vector<int> batch;
        batch.reserve(1024);

        int expanded = 0;
        bool found   = false;

        while (!openList.empty() && !found) {

            // Step 1: collect all nodes at minimum f [SERIAL]
            batch.clear();
            int minF = openList.top().first;
            while (!openList.empty() && openList.top().first == minF) {
                auto [f, idx] = openList.top(); openList.pop();
                bool alreadyClosed = false;
                if (closed[idx].compare_exchange_strong(alreadyClosed, true,
                        memory_order_acq_rel, memory_order_relaxed)) {
                    batch.push_back(idx);
                    ++expanded;
                    if (idx == goalIdx) { found = true; break; }
                }
            }
            if (found) break;
            if (batch.empty()) continue;

            // Step 2: expand batch in parallel [PARALLEL]
            #pragma omp parallel num_threads(T)
            {
                int tid = omp_get_thread_num();
                auto& myBuf = localBuf[tid];

                #pragma omp for schedule(static)
                for (int bi = 0; bi < (int)batch.size(); ++bi) {
                    int idx = batch[bi];
                    int cx  = idx / W, cy = idx % W;
                    int cg  = gScore[idx].load(memory_order_relaxed);

                    for (int i = 0; i < NUM_DIRECTIONS; ++i) {
                        int nx = cx+DX[i], ny = cy+DY[i];
                        if (!grid.isValid(nx, ny)) continue;
                        int nIdx = nx*W + ny;
                        if (closed[nIdx].load(memory_order_relaxed)) continue;

                        int ng = cg + 1;
                        if (atomicMin(gScore[nIdx], ng)) {
                            parent[nIdx] = idx;
                            int nf = ng + h(nx, ny);
                            myBuf.push_back({nf, nIdx, idx, ng});
                        }
                    }
                }
            }

            // Step 3: merge discoveries into openList [SERIAL]
            for (auto& buf : localBuf) {
                for (auto& [f, nIdx, parIdx, ng] : buf) {
                    if (gScore[nIdx].load(memory_order_relaxed) == ng)
                        openList.push({f, nIdx});
                }
                buf.clear();
            }
        }

        cout << "Nodes expanded: " << expanded << endl;

        vector<pair<int,int>> path;
        if (found) {
            int idx = goalIdx;
            while (idx != -2) { path.push_back({idx/W, idx%W}); idx = parent[idx]; }
            reverse(path.begin(), path.end());
        }
        return path;
    }
};

int main(int argc, char* argv[]) {
    int W=1000, H=1000;
    double density=0.3;
    unsigned int seed=42;
    int numThreads=omp_get_max_threads();

    if(argc>=3){W=atoi(argv[1]); H=atoi(argv[2]);}
    if(argc>=4) density=atof(argv[3]);
    if(argc>=5) seed=atoi(argv[4]);
    if(argc>=6) numThreads=atoi(argv[5]);
    omp_set_num_threads(numThreads);

    cout<<"========================================"<<endl;
    cout<<"OpenMP Parallel A*"<<endl;
    cout<<"Grid size: "<<W<<"x"<<H<<endl;
    cout<<"Obstacle density: "<<(density*100)<<"%"<<endl;
    cout<<"Threads: "<<numThreads<<endl;
    cout<<"========================================"<<endl;

    Grid grid(W,H);
    grid.generateObstacles(density,seed);
    int startX=0,startY=0,goalX=H-1,goalY=W-1;
    grid.clearCell(startX,startY);
    grid.clearCell(goalX,goalY);
    cout<<"Start: ("<<startX<<","<<startY<<") | Goal: ("<<goalX<<","<<goalY<<")"<<endl;

    AStarOpenMP astar(grid,startX,startY,goalX,goalY,numThreads);
    Timer timer; timer.start();
    auto path = astar.findPath();
    double t = timer.stop();

    cout<<"========================================"<<endl;
    if(path.empty()) cout<<"No path found!"<<endl;
    else {
        cout<<"Path found! Length: "<<path.size()<<" steps"<<endl;
        printPath(path);
    }
    cout<<"Execution time: "<<t<<" ms"<<endl;
    cout<<"Threads used:   "<<numThreads<<endl;
    cout<<"========================================"<<endl;

    saveResults("results/performance_logs.txt","openmp",W,H,numThreads,1,t,path.size());
    return 0;
}
