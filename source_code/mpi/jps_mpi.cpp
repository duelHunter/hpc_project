#include <mpi.h>
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

// MPI Message structure
struct Msg {
    int jp;
    int g;
    int pIdx;
};

class JPSMPI {
    Grid& grid;
    vector<uint8_t> flatGrid;
    int sx, sy, gx, gy, H, W;
    int rank, num_procs;
    int row_chunk;

    inline int dist(int x, int y) const { 
        return max(abs(x - gx), abs(y - gy)); 
    }

    inline bool rawValid(int x, int y) const {
        return (unsigned)x < (unsigned)H && (unsigned)y < (unsigned)W && flatGrid[x * W + y] == 0;
    }

    // Identifies which MPI rank owns a specific cell based on its row (X)
    inline int getOwner(int x) const {
        return x / row_chunk;
    }

    int jump(int x, int y, int dx, int dy) const {
        int nx = x, ny = y;
        while (true) {
            nx += dx;
            ny += dy;
            if (!rawValid(nx, ny)) return -1;
            if (nx == gx && ny == gy) return nx * W + ny;
            
            if (dx != 0 && dy != 0) {
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
    JPSMPI(Grid& g, int sx, int sy, int gx, int gy, int r, int procs)
        : grid(g), sx(sx), sy(sy), gx(gx), gy(gy), H(g.getHeight()), W(g.getWidth()), rank(r), num_procs(procs) {
        flatGrid.assign(H * W, 0);
        for(int x=0; x<H; ++x) {
            for(int y=0; y<W; ++y) {
                flatGrid[x * W + y] = grid.getCell(x, y);
            }
        }
        row_chunk = (H + num_procs - 1) / num_procs;
    }

    vector<pair<int, int>> findPath() {
        vector<int> gScore(H * W, INT_MAX);
        vector<int> parent(H * W, -3); // -3 means unvisited by any rank
        vector<bool> closed(H * W, false);

        using PQ = pair<int, int>;
        priority_queue<PQ, vector<PQ>, greater<PQ>> openList;

        long long local_expanded = 0;
        int local_found = 0; // 0 or 1
        
        // Define Custom MPI Type for Msg
        MPI_Datatype MPI_MSG_TYPE;
        int blocklengths[3] = {1, 1, 1};
        MPI_Aint displacements[3] = {offsetof(Msg, jp), offsetof(Msg, g), offsetof(Msg, pIdx)};
        MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
        MPI_Type_create_struct(3, blocklengths, displacements, types, &MPI_MSG_TYPE);
        MPI_Type_commit(&MPI_MSG_TYPE);

        int startIdx = sx * W + sy;
        if (getOwner(sx) == rank) {
            gScore[startIdx] = 0;
            parent[startIdx] = -2;
            openList.push({dist(sx, sy), startIdx});
        }

        vector<vector<Msg>> send_buffers(num_procs);
        vector<int> send_counts(num_procs, 0);
        vector<int> recv_counts(num_procs, 0);
        vector<int> send_displs(num_procs, 0);
        vector<int> recv_displs(num_procs, 0);
        vector<Msg> flat_send_buf;
        vector<Msg> flat_recv_buf;

        int global_found = 0;
        int active = 1;

        while (active) {
            // Clear send buffers
            for (int p=0; p<num_procs; ++p) send_buffers[p].clear();
            
            // Phase 1: Expand local nodes (Batch mapping)
            int batchSize = 2048; // Process a set number of items before communicating
            int expanded_this_round = 0;
            
            while (!openList.empty() && expanded_this_round < batchSize) {
                auto [f, idx] = openList.top();
                openList.pop();

                if (closed[idx]) continue;
                closed[idx] = true;
                ++local_expanded;
                ++expanded_this_round;

                if (idx == gx * W + gy) {
                    local_found = 1;
                    continue; // Dont break immediately so we notify others
                }

                int cx = idx / W, cy = idx % W;
                int cg = gScore[idx];

                for (int i = 0; i < 8; ++i) {
                    int jp = jump(cx, cy, jdx[i], jdy[i]);
                    
                    if (jp != -1) {
                        int jx = jp / W, jy = jp % W;
                        int d = max(abs(cx - jx), abs(cy - jy));
                        int ng = cg + d;
                        
                        int owner = getOwner(jx);
                        if (owner == rank) {
                            if (ng < gScore[jp]) {
                                gScore[jp] = ng;
                                parent[jp] = idx;
                                closed[jp] = false;
                                openList.push({ng + dist(jx, jy), jp});
                            }
                        } else {
                            send_buffers[owner].push_back({jp, ng, idx});
                        }
                    }
                }
            }

            // Phase 2: Communication (Bulk Synchronous)
            for (int p=0; p<num_procs; ++p) send_counts[p] = send_buffers[p].size();
            
            // Exchange counts
            MPI_Alltoall(send_counts.data(), 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

            // Compute displacements
            int total_send = 0;
            int total_recv = 0;
            for (int p=0; p<num_procs; ++p) {
                send_displs[p] = total_send;
                total_send += send_counts[p];
                recv_displs[p] = total_recv;
                total_recv += recv_counts[p];
            }

            flat_send_buf.resize(total_send);
            int k = 0;
            for (int p=0; p<num_procs; ++p) {
                for (const auto& msg : send_buffers[p]) {
                    flat_send_buf[k++] = msg;
                }
            }
            flat_recv_buf.resize(total_recv);

            // Exchange data
            MPI_Alltoallv(flat_send_buf.data(), send_counts.data(), send_displs.data(), MPI_MSG_TYPE,
                          flat_recv_buf.data(), recv_counts.data(), recv_displs.data(), MPI_MSG_TYPE, MPI_COMM_WORLD);

            // Phase 3: Process received messages
            for (int i=0; i<total_recv; ++i) {
                Msg m = flat_recv_buf[i];
                if (m.g < gScore[m.jp]) {
                    gScore[m.jp] = m.g;
                    parent[m.jp] = m.pIdx;
                    closed[m.jp] = false;
                    int jx = m.jp / W, jy = m.jp % W;
                    openList.push({m.g + dist(jx, jy), m.jp});
                }
            }

            // Phase 4: Termination Check
            int local_has_work = (!openList.empty());
            int global_has_work = 0;
            MPI_Allreduce(&local_has_work, &global_has_work, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            MPI_Allreduce(&local_found, &global_found, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

            // We are active if anyone has work AND we haven't found the goal
            active = (global_has_work > 0) && (global_found == 0);
        }

        // We need to print metrics perfectly once
        long long global_expanded = 0;
        MPI_Reduce(&local_expanded, &global_expanded, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            cout << "Nodes expanded (all processes): " << global_expanded << endl;
        }

        // Gather all parents to reconstruct path on Rank 0
        vector<int> global_parent;
        if (rank == 0) {
            global_parent.resize(H * W);
        }
        
        // Use MPI_MAX to combine the parent arrays. 
        // Initializing with -3 means nodes untouched anywhere will remain -3.
        // Valid nodes will be >= -2.
        MPI_Reduce(parent.data(), global_parent.data(), H * W, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

        vector<pair<int, int>> path;
        
        if (rank == 0 && global_found) {
            int curr = gx * W + gy;
            vector<pair<int, int>> points;
            
            // Reconstruct logic
            while (curr != -2 && curr >= 0) {
                points.push_back({curr / W, curr % W});
                curr = global_parent[curr];
            }
            // Add Start
            if (curr == -2) {
                points.push_back({sx, sy}); 
            }
            reverse(points.begin(), points.end());
            
            // Interpolate path for continuous steps
            if (points.size() > 0) {
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
        }
        
        MPI_Type_free(&MPI_MSG_TYPE);
        return path;
    }
};

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    int W=1000, H=1000; double density=0.3; unsigned int seed=42;
    
    if(argc>=3){W=atoi(argv[1]); H=atoi(argv[2]);}
    if(argc>=4) density=atof(argv[3]);
    if(argc>=5) seed=atoi(argv[4]);

    if (rank == 0) {
        cout<<"========================================"<<endl;
        cout<<"MPI Distributed JPS Pathfinding"<<endl;
        cout<<"Grid size: "<<W<<"x"<<H<<endl;
        cout<<"Obstacle density: "<<(density*100)<<"%"<<endl;
        cout<<"Processes: "<<procs<<endl;
        cout<<"========================================"<<endl;
    }

    Grid grid(W,H);
    grid.generateObstacles(density,seed);
    int startX=0,startY=0,goalX=H-1,goalY=W-1;
    grid.clearCell(startX,startY);
    grid.clearCell(goalX,goalY);
    
    if (rank == 0) {
        cout<<"Start: ("<<startX<<","<<startY<<") | Goal: ("<<goalX<<","<<goalY<<")"<<endl;
    }

    JPSMPI jps(grid,startX,startY,goalX,goalY,rank,procs);
    
    MPI_Barrier(MPI_COMM_WORLD);
    Timer timer; 
    if (rank == 0) timer.start();
    
    auto path = jps.findPath();
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        double t = timer.stop();
        cout<<"========================================"<<endl;
        if(path.empty()) cout<<"No path found!"<<endl;
        else{
            cout<<"Path found! Length: "<<path.size()<<" steps"<<endl;
        }
        cout<<"Execution time: "<<t<<" ms"<<endl;
        cout<<"========================================"<<endl;
        
        saveResults("results/performance_logs.txt","mpi_jps",W,H,procs,1,t,path.size());
        if(W<=100 && H<=100){
            exportGridWithPath("results/mpi_jps_path.txt",grid.getData(),path,startX,startY,goalX,goalY);
            exportGridWithPathSVG("results/mpi_jps_path.svg",grid.getData(),path,startX,startY,goalX,goalY);
        }
    }
    
    MPI_Finalize();
    return 0;
}
