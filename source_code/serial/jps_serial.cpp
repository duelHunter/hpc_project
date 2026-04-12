#include <iostream>
#include <vector>
#include <queue>
#include <tuple>
#include <algorithm>
#include <climits>
#include <cmath>
#include "../common/grid.h"
#include "../common/utils.h"

using namespace std;

// 8 directions: N, S, W, E, NW, NE, SW, SE
const int jdx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
const int jdy[] = {0, 0, -1, 1, -1, 1, -1, 1};

class JPSSerial {
    Grid& grid;
    int sx, sy, gx, gy, H, W;
    
    // Diagonal distance (Chebyshev)
    inline int dist(int x, int y) const { 
        return max(abs(x - gx), abs(y - gy)); 
    }

    inline bool isValid(int x, int y) const {
        return grid.isValid(x, y);
    }
    
    // Jump Point Search core function - Iterative for performance
    int jump(int x, int y, int dx, int dy) {
        int nx = x;
        int ny = y;
        
        while (true) {
            nx += dx;
            ny += dy;
            
            if (!isValid(nx, ny)) return -1;
            if (nx == gx && ny == gy) return nx * W + ny;
            
            if (dx != 0 && dy != 0) { // Diagonal movement
                if ((isValid(nx - dx, ny + dy) && !isValid(nx - dx, ny)) || 
                    (isValid(nx + dx, ny - dy) && !isValid(nx, ny - dy))) {
                    return nx * W + ny; // Forced neighbor found
                }
                
                // Recursively check orthogonal directions
                if (jump(nx, ny, dx, 0) != -1 || jump(nx, ny, 0, dy) != -1) {
                    return nx * W + ny;
                }
            } else {
                if (dx != 0) { // Horizontal movement
                    if ((isValid(nx + dx, ny + 1) && !isValid(nx, ny + 1)) || 
                        (isValid(nx + dx, ny - 1) && !isValid(nx, ny - 1))) {
                        return nx * W + ny;
                    }
                } else { // Vertical movement
                    if ((isValid(nx + 1, ny + dy) && !isValid(nx + 1, ny)) || 
                        (isValid(nx - 1, ny + dy) && !isValid(nx - 1, ny))) {
                        return nx * W + ny;
                    }
                }
            }
        }
        return -1;
    }

public:
    JPSSerial(Grid& g, int sx, int sy, int gx, int gy)
        : grid(g), sx(sx), sy(sy), gx(gx), gy(gy), H(g.getHeight()), W(g.getWidth()) {}

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

        while (!openList.empty()) {
            auto [f, idx] = openList.top();
            openList.pop();

            if (closed[idx]) continue;
            closed[idx] = true;
            ++expanded;

            int cx = idx / W, cy = idx % W;
            if (cx == gx && cy == gy) {
                found = true;
                break;
            }

            int cg = gScore[idx];

            for (int i = 0; i < 8; ++i) {
                int jp = jump(cx, cy, jdx[i], jdy[i]);
                
                if (jp != -1 && !closed[jp]) {
                    int jx = jp / W, jy = jp % W;
                    int d = max(abs(cx - jx), abs(cy - jy));
                    int ng = cg + d;
                    
                    if (ng < gScore[jp]) {
                        gScore[jp] = ng;
                        parent[jp] = idx;
                        openList.push({ng + dist(jx, jy), jp});
                    }
                }
            }
        }

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
    if(argc>=3){W=atoi(argv[1]); H=atoi(argv[2]);}
    if(argc>=4) density=atof(argv[3]);
    if(argc>=5) seed=atoi(argv[4]);

    cout<<"========================================"<<endl;
    cout<<"Serial JPS Pathfinding"<<endl;
    cout<<"Grid size: "<<W<<"x"<<H<<endl;
    cout<<"Obstacle density: "<<(density*100)<<"%"<<endl;
    cout<<"========================================"<<endl;

    Grid grid(W,H);
    grid.generateObstacles(density,seed);
    int startX=0,startY=0,goalX=H-1,goalY=W-1;
    grid.clearCell(startX,startY);
    grid.clearCell(goalX,goalY);
    cout<<"Start: ("<<startX<<","<<startY<<") | Goal: ("<<goalX<<","<<goalY<<")"<<endl;

    JPSSerial jps(grid,startX,startY,goalX,goalY);
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
    
    saveResults("results/performance_logs.txt","serial_jps",W,H,1,1,t,path.size());
    if(W<=100 && H<=100){
        exportGridWithPath("results/serial_jps_path.txt",grid.getData(),path,startX,startY,goalX,goalY);
        exportGridWithPathSVG("results/serial_jps_path.svg",grid.getData(),path,startX,startY,goalX,goalY);
    }
    return 0;
}
