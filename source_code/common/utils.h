#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
public:
    void start() { start_time = std::chrono::high_resolution_clock::now(); }
    double elapsed() {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> d = end_time - start_time;
        return d.count();
    }
    double stop() { return elapsed(); }
};

const int DX[] = {-1, 1, 0, 0};
const int DY[] = {0, 0, -1, 1};
const int NUM_DIRECTIONS = 4;

inline void printPath(const std::vector<std::pair<int,int>>& path) {
    std::cout << "Path found with " << path.size() << " steps:" << std::endl;
    size_t lim = std::min(path.size(), size_t(10));
    for (size_t i = 0; i < lim; ++i) {
        std::cout << "(" << path[i].first << "," << path[i].second << ")";
        if (i < lim - 1) std::cout << " -> ";
    }
    if (path.size() > 10)
        std::cout << " ... (" << path.back().first << "," << path.back().second << ")";
    std::cout << std::endl;
}

inline void saveResults(const std::string& filename, const std::string& version,
                        int w, int h, int threads, int procs,
                        double execTime, int pathLen) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open())
        file << version << "," << w << "x" << h << "," << threads << ","
             << procs << "," << execTime << "," << pathLen << std::endl;
    else
        std::cerr << "Unable to open results file" << std::endl;
}

inline void exportGridWithPath(const std::string& filename,
                               const std::vector<std::vector<int>>& grid,
                               const std::vector<std::pair<int,int>>& path,
                               int sx, int sy, int gx, int gy) {
    std::ofstream file(filename);
    if (!file.is_open()) { std::cerr << "Cannot open: " << filename << std::endl; return; }
    int H = grid.size(), W = grid[0].size();
    std::vector<std::vector<char>> vis(H, std::vector<char>(W));
    for (int i = 0; i < H; ++i)
        for (int j = 0; j < W; ++j)
            vis[i][j] = (grid[i][j] == 1) ? 'X' : '.';
    for (auto& p : path)
        if (p.first >= 0 && p.first < H && p.second >= 0 && p.second < W)
            vis[p.first][p.second] = '*';
    vis[sx][sy] = 'S';
    vis[gx][gy] = 'G';
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) file << vis[i][j] << " ";
        file << "\n";
        file << "\n";
    }
    std::cout << "Grid exported to: " << filename << std::endl;
}

#endif
