#include <iostream>
#include <vector>
#include <queue>
#include <tuple>
#include <algorithm>
#include <climits>
#include <omp.h>
#include "../common/grid.h"
#include "../common/utils.h"

using namespace std;

class AStarOpenMP {
    Grid& grid;
    int sx, sy, gx, gy, H, W;
    int numThreads;
    inline int h(int x, int y) const { return abs(x-gx)+abs(y-gy); }
public:
    AStarOpenMP(Grid& g,int sx,int sy,int gx,int gy,int threads)
        :grid(g),sx(sx),sy(sy),gx(gx),gy(gy),
         H(g.getHeight()),W(g.getWidth()),numThreads(threads){}

    vector<pair<int,int>> findPath(){
        vector<int> gScore(H*W, INT_MAX);
        vector<int> parent(H*W, -1);
        vector<bool> closed(H*W, false);

        using PQ = pair<int,int>;
        priority_queue<PQ,vector<PQ>,greater<PQ>> openList;

        int startIdx=sx*W+sy;
        gScore[startIdx]=0;
        parent[startIdx]=-2;
        openList.push({h(sx,sy), startIdx});

        // Batch size: scale with thread count so every thread gets work
        const int BATCH = numThreads * 4;

        int expanded=0;
        bool found=false;
        int goalIdx=gx*W+gy;

        while(!openList.empty() && !found){
            // -- collect a batch of the best nodes from the queue ----
            vector<int> batch;
            batch.reserve(BATCH);
            while(!openList.empty() && (int)batch.size()<BATCH){
                auto [f,idx] = openList.top(); openList.pop();
                if(closed[idx]) continue;
                closed[idx]=true;
                ++expanded;
                batch.push_back(idx);
                if(idx==goalIdx){ found=true; break; }
            }
            if(found) break;

            // -- expand each node in the batch in parallel ------------
            // Each thread collects its generated neighbours locally,
            // then one critical section merges them into the global queue.
            #pragma omp parallel num_threads(numThreads)
            {
                // thread-local buffer: (f_cost, nIdx, ng, parentIdx)
                vector<tuple<int,int,int,int>> localGen;
                localGen.reserve(BATCH*4);

                #pragma omp for schedule(dynamic,1)
                for(int b=0; b<(int)batch.size(); ++b){
                    int idx=batch[b];
                    int cx=idx/W, cy=idx%W;
                    int cg=gScore[idx];

                    for(int i=0;i<NUM_DIRECTIONS;++i){
                        int nx=cx+DX[i], ny=cy+DY[i];
                        if(!grid.isValid(nx,ny)) continue;
                        int nIdx=nx*W+ny;
                        if(closed[nIdx]) continue;
                        int ng=cg+1;
                        // optimistic check before locking
                        if(ng < gScore[nIdx]){
                            localGen.emplace_back(ng+h(nx,ny), nIdx, ng, idx);
                        }
                    }
                }

                // merge thread-local results into global structures
                #pragma omp critical
                {
                    for(auto& [f,nIdx,ng,pIdx] : localGen){
                        if(!closed[nIdx] && ng < gScore[nIdx]){
                            gScore[nIdx]=ng;
                            parent[nIdx]=pIdx;
                            openList.push({f, nIdx});
                        }
                    }
                }
            } // end parallel
        }

        cout<<"Nodes expanded: "<<expanded<<endl;
        vector<pair<int,int>> path;
        if(found){
            int idx=goalIdx;
            while(idx!=-2){ path.push_back({idx/W,idx%W}); idx=parent[idx]; }
            reverse(path.begin(),path.end());
        }
        return path;
    }
};

int main(int argc,char* argv[]){
    int W=1000,H=1000; double density=0.3; unsigned int seed=42;
    int numThreads=omp_get_max_threads();

    if(argc>=3){W=atoi(argv[1]);H=atoi(argv[2]);}
    if(argc>=4) density=atof(argv[3]);
    if(argc>=5) seed=atoi(argv[4]);
    if(argc>=6) numThreads=atoi(argv[5]);

    omp_set_num_threads(numThreads);

    cout<<"========================================"<<endl;
    cout<<"OpenMP Parallel A* Pathfinding"<<endl;
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
    auto path=astar.findPath();
    double t=timer.stop();

    cout<<"========================================"<<endl;
    if(path.empty()) cout<<"No path found!"<<endl;
    else{
        cout<<"Path found! Length: "<<path.size()<<" steps"<<endl;
        printPath(path);
    }
    cout<<"Execution time: "<<t<<" ms"<<endl;
    cout<<"Threads used: "<<numThreads<<endl;
    cout<<"========================================"<<endl;
    saveResults("../results/performance_logs.txt","openmp",W,H,numThreads,1,t,path.size());
    return 0;
}