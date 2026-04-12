#include <iostream>
#include <vector>
#include <queue>
#include <tuple>
#include <algorithm>
#include <climits>
#include <cmath>
#include <omp.h>
#include "../common/grid.h"
#include "../common/utils.h"

using namespace std;

// 8 directions: N, S, W, E, NW, NE, SW, SE
const int jdx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
const int jdy[] = {0, 0, -1, 1, -1, 1, -1, 1};

struct JumpResult {
    int f, jp, ng, pIdx;
};

class JPSOpenMP {
    Grid& grid;
    int sx, sy, gx, gy, H, W, T;
    
    // Diagonal distance (Chebyshev)
    inline int dist(int x, int y) const { 
        return max(abs(x - gx), abs(y - gy)); 
    }

    inline bool isValid(int x, int y) const {
        return grid.isValid(x, y);
    }
    
    
    inline bool rawValid(int x, int y) const {
        return grid.isValid(x, y);
    }
    
    // Optimized Iterative Jump logic completely unrolled
    int jump(int x, int y, int dx, int dy) const {
        int nx = x, ny = y;
        
        while (true) {
            nx += dx;
            ny += dy;
            
            if (!rawValid(nx, ny)) return -1;
            if (nx == gx && ny == gy) return nx * W + ny;
            
            if (dx != 0 && dy != 0) { // Diagonal movement
                if ((rawValid(nx - dx, ny + dy) && !rawValid(nx - dx, ny)) || 
                    (rawValid(nx + dx, ny - dy) && !rawValid(nx, ny - dy))) {
                    return nx * W + ny;
                }
                
                if (jumpSearchOrthogonal(nx, ny, dx, 0) || jumpSearchOrthogonal(nx, ny, 0, dy)) {
                    return nx * W + ny;
                }
            } else {
                if (dx != 0) {
                    if ((rawValid(nx + dx, ny + 1) && !rawValid(nx, ny + 1)) || 
                        (rawValid(nx + dx, ny - 1) && !rawValid(nx, ny - 1))) return nx * W + ny;
                } else {
                    if ((rawValid(nx + 1, ny + dy) && !rawValid(nx + 1, ny)) || 
                        (rawValid(nx - 1, ny + dy) && !rawValid(nx - 1, ny))) return nx * W + ny;
                }
            }
        }
        return -1;
    }

    bool jumpSearchOrthogonal(int nx, int ny, int dx, int dy) const {
        while (true) {
            nx += dx;
            ny += dy;
            if (!rawValid(nx, ny)) return false;
            if (nx == gx && ny == gy) return true;
            if (dx != 0) {
                if ((rawValid(nx + dx, ny + 1) && !rawValid(nx, ny + 1)) || 
                    (rawValid(nx + dx, ny - 1) && !rawValid(nx, ny - 1))) return true;
            } else {
                if ((rawValid(nx + 1, ny + dy) && !rawValid(nx + 1, ny)) || 
                    (rawValid(nx - 1, ny + dy) && !rawValid(nx - 1, ny))) return true;
            }
        }
        return false;
    }

public:
    JPSOpenMP(Grid& g, int sx, int sy, int gx, int gy, int threads)
        : grid(g), sx(sx), sy(sy), gx(gx), gy(gy), H(g.getHeight()), W(g.getWidth()), T(threads) {}

    vector<pair<int, int>> findPath() {
        vector<int> gScore(H * W, INT_MAX);
        vector<int> parent(H * W, -1);
        vector<bool> closed(H * W, false);

        using PQ = pair<int, int>;
        priority_queue<PQ, vector<PQ>, greater<PQ>> openList;

        int startIdx = sx * W + sy;
        gScore[startIdx] = 0;
        parent[startIdx] = -2;
        openList.push({dist(sx, sy), startIdx});

        int expanded = 0;
        bool found = false;
        
        vector<int> batch;
        batch.reserve(4096);

        // Hoist thread creation to occur exactly ONCE. Massive performance gain!
        #pragma omp parallel num_threads(T)
        {
            vector<JumpResult> localGen;
            localGen.reserve(1024); // Thread-local buffer survives the entire search
            
            while (true) {
                #pragma omp single
                {
                    batch.clear();
                    if (!openList.empty() && !found) {
                        int min_f = openList.top().first;
                        int batchLimit = max(256, T * 32); // larger batch limit
                        
                        while (!openList.empty() && batch.size() < (size_t)batchLimit && (openList.top().first <= min_f + 5)) {
                            auto [f, idx] = openList.top();
                            openList.pop();

                            if (closed[idx]) continue;
                            closed[idx] = true;
                            ++expanded;

                            if (idx == gx * W + gy) {
                                found = true;
                                break;
                            }
                            
                            batch.push_back(idx);
                        }
                    }
                } // Implicit barrier here ensures all threads see the new batch and found values

                if (found || batch.empty()) break;

                localGen.clear();
                
                #pragma omp for schedule(static) nowait
                for (int b = 0; b < (int)batch.size(); ++b) {
                    int idx = batch[b];
                    int cx = idx / W, cy = idx % W;
                    int cg = gScore[idx];

                    for (int i = 0; i < 8; ++i) {
                        int jp = jump(cx, cy, jdx[i], jdy[i]);
                        
                        if (jp != -1) {
                            int jx = jp / W, jy = jp % W;
                            int d = max(abs(cx - jx), abs(cy - jy));
                            int ng = cg + d;
                            
                            if (ng < gScore[jp]) {
                                localGen.push_back({ng + dist(jx, jy), jp, ng, idx});
                            }
                        }
                    }
                }

                // Critical section merged in bulk for safety but optimized
                #pragma omp critical
                {
                    for (auto& res : localGen) {
                        if (res.ng < gScore[res.jp]) {
                            gScore[res.jp] = res.ng;
                            parent[res.jp] = res.pIdx;
                            closed[res.jp] = false; 
                            openList.push({res.f, res.jp});
                        }
                    }
                }
                
                // Explicit barrier to ensure all pushes to openList are complete before the master thread pops the next batch
                #pragma omp barrier
            }
        } // End of parallel region

        cout << "Nodes expanded: " << expanded << endl;
        vector<pair<int, int>> path;
        
        if (found) {
            int curr = gx * W + gy;
            vector<pair<int, int>> points;
            while (curr != -2) {
                points.push_back({curr / W, curr % W});
                curr = parent[curr];
            }
            reverse(points.begin(), points.end());
            
            // Interpolate path for continuous steps
            for(size_t i = 0; i < points.size() - 1; i++) {
                int c_x = points[i].first, c_y = points[i].second;
                int n_x = points[i+1].first, n_y = points[i+1].second;
                
                int dir_x = (n_x > c_x) ? 1 : (n_x < c_x ? -1 : 0);
                int dir_y = (n_y > c_y) ? 1 : (n_y < c_y ? -1 : 0);
                
                while(c_x != n_x || c_y != n_y) {
                    path.push_back({c_x, c_y});
                    c_x += dir_x;
                    c_y += dir_y;
                }
            }
            path.push_back(points.back());
        }
        return path;
    }
};

int main(int argc, char* argv[]) {
    int W=1000, H=1000; double density=0.3; unsigned int seed=42;
    int numThreads = omp_get_max_threads();
    
    if(argc>=3){W=atoi(argv[1]); H=atoi(argv[2]);}
    if(argc>=4) density=atof(argv[3]);
    if(argc>=5) seed=atoi(argv[4]);
    if(argc>=6) numThreads=atoi(argv[5]);
    
    omp_set_num_threads(numThreads);

    cout<<"========================================"<<endl;
    cout<<"OpenMP Parallel JPS Pathfinding"<<endl;
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

    JPSOpenMP jps(grid,startX,startY,goalX,goalY,numThreads);
    Timer timer; timer.start();
    auto path = jps.findPath();
    double t = timer.stop();

    cout<<"========================================"<<endl;
    if(path.empty()) cout<<"No path found!"<<endl;
    else{
        cout<<"Path found! Length: "<<path.size()<<" steps"<<endl;
        printPath(path);
    }
    cout<<"Execution time: "<<t<<" ms"<<endl;
    cout<<"========================================"<<endl;
    
    saveResults("results/performance_logs.txt","openmp_jps",W,H,numThreads,1,t,path.size());
    if(W<=100 && H<=100){
        exportGridWithPath("results/openmp_jps_path.txt",grid.getData(),path,startX,startY,goalX,goalY);
        exportGridWithPathSVG("results/openmp_jps_path.svg",grid.getData(),path,startX,startY,goalX,goalY);
    }
    return 0;
}
