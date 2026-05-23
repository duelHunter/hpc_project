#include <iostream>
#include <vector>
#include <queue>
#include <tuple>
#include <algorithm>
#include <climits>
#include "../common/grid.h"
#include "../common/utils.h"

using namespace std;

class AStarSerial {
    Grid& grid;
    int sx, sy, gx, gy, H, W;
    inline int h(int x, int y) const { return abs(x-gx)+abs(y-gy); }
public:
    AStarSerial(Grid& g,int sx,int sy,int gx,int gy)
        :grid(g),sx(sx),sy(sy),gx(gx),gy(gy),H(g.getHeight()),W(g.getWidth()){}

    ////////////////////////Main A* pathfinding function
    vector<pair<int,int>> findPath(){
        vector<int> gScore(H*W, INT_MAX);
        vector<int> parent(H*W, -1);
        vector<bool> closed(H*W, false);

        using PQ = pair<int,int>;
        priority_queue<PQ,vector<PQ>,greater<PQ>> openList;

        int startIdx = sx*W+sy;
        gScore[startIdx] = 0;
        parent[startIdx] = -2;
        openList.push({h(sx,sy), startIdx});

        int expanded = 0;
        bool found = false;

        while(!openList.empty()){
            auto [f, idx] = openList.top(); openList.pop();
            if(closed[idx]) continue;
            closed[idx] = true;
            ++expanded;
            int cx=idx/W, cy=idx%W;
            if(cx==gx && cy==gy){ found=true; break; }
            int cg = gScore[idx];
            #pragma omp parallel for schedule(dynamic,1)
            for(int i=0;i<NUM_DIRECTIONS;++i){
                int nx=cx+DX[i], ny=cy+DY[i];
                if(!grid.isValid(nx,ny)) continue;
                int nIdx=nx*W+ny;
                if(closed[nIdx]) continue;
                int ng=cg+1;
                if(ng < gScore[nIdx]){
                    gScore[nIdx]=ng;
                    parent[nIdx]=idx;
                    openList.push({ng+h(nx,ny), nIdx});
                }
            }
        }
        cout << "Nodes expanded: " << expanded << endl;
        vector<pair<int,int>> path;
        if(found){
            int idx=gx*W+gy;
            while(idx!=-2){ path.push_back({idx/W,idx%W}); idx=parent[idx]; }
            reverse(path.begin(),path.end());
        }
        return path;
    }
};

int main(int argc, char* argv[]){
    int W=10000,H=10000; double density=0.2; unsigned int seed=42;
    if(argc>=3){W=atoi(argv[1]);H=atoi(argv[2]);}
    if(argc>=4) density=atof(argv[3]);
    if(argc>=5) seed=atoi(argv[4]);

    cout<<"========================================"<<endl;
    cout<<"Serial A* Pathfinding"<<endl;
    cout<<"Grid size: "<<W<<"x"<<H<<endl;
    cout<<"Obstacle density: "<<(density*100)<<"%"<<endl;
    cout<<"========================================"<<endl;

    Grid grid(W,H);
    grid.generateObstacles(density,seed);
    int startX=0,startY=0,goalX=H-1,goalY=W-1;
    grid.clearCell(startX,startY);
    grid.clearCell(goalX,goalY);
    cout<<"Start: ("<<startX<<","<<startY<<") | Goal: ("<<goalX<<","<<goalY<<")"<<endl;

    AStarSerial astar(grid,startX,startY,goalX,goalY);
    Timer timer; timer.start();
    auto path=astar.findPath();
    double t=timer.stop();

    cout<<"========================================"<<endl;
    if(path.empty()) cout<<"No path found!"<<endl;
    else{
        cout<<"Path found! Length: "<<path.size()<<" steps"<<endl;
        printPath(path);
    }
    cout<<"Execution time: "<<t<<" ms"<<endl;
    cout<<"========================================"<<endl;
    saveResults("results/performance_logs.txt","serial",W,H,1,1,t,path.size());
    // if(W<=10000&&H<=10000){
    //     exportGridWithPath("results/serial_path.txt",grid.getData(),path,startX,startY,goalX,goalY);
    //     exportGridWithPathSVG("results/serial_path.svg",grid.getData(),path,startX,startY,goalX,goalY);
    // }
    return 0;
}
