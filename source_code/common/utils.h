#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <iostream>
#include <vector>
#include <fstream>

/**
 * Performance Timer utility
 */
class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    
public:
    // Start the timer
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    // Get elapsed time in milliseconds
    double elapsed() {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;
        return duration.count();
    }
    
    // Stop and return elapsed time
    double stop() {
        return elapsed();
    }
};

/**
 * Direction vectors for 4-directional movement
 * Up, Down, Left, Right
 */
const int DX[] = {-1, 1, 0, 0};
const int DY[] = {0, 0, -1, 1};
const int NUM_DIRECTIONS = 4;

/**
 * Print path to console
 */
void printPath(const std::vector<std::pair<int, int>>& path) {
    std::cout << \"Path found with \" << path.size() << \" steps:\" << std::endl;
    for (size_t i = 0; i < std::min(path.size(), size_t(10)); ++i) {
        std::cout << \"(\" << path[i].first << \", \" << path[i].second << \")\";
        if (i < std::min(path.size(), size_t(10)) - 1) std::cout << \" -> \";
    }
    if (path.size() > 10) {
        std::cout << \" ... (\" << path.back().first << \", \" << path.back().second << \")\";
    }
    std::cout << std::endl;
}

/**
 * Save results to file
 */
void saveResults(const std::string& filename, const std::string& version,
                 int gridWidth, int gridHeight, int threads, int processes,
                 double executionTime, int pathLength) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << version << \",\" 
             << gridWidth << \"x\" << gridHeight << \",\"
             << threads << \",\"
             << processes << \",\"
             << executionTime << \",\"
             << pathLength << std::endl;
        file.close();
    } else {
        std::cerr << \"Unable to open results file\" << std::endl;
    }
}

/**
 * Export grid and path to file for visualization
 */
void exportGridWithPath(const std::string& filename, 
                        const std::vector<std::vector<int>>& grid,
                        const std::vector<std::pair<int, int>>& path,
                        int startX, int startY, int goalX, int goalY) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << \"Cannot open file for export: \" << filename << std::endl;
        return;
    }
    
    int height = grid.size();
    int width = grid[0].size();
    
    // Create a copy of grid for visualization
    std::vector<std::vector<char>> visualGrid(height, std::vector<char>(width));
    
    // Fill with grid data
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            visualGrid[i][j] = (grid[i][j] == 1) ? 'X' : '.';
        }
    }
    
    // Mark path
    for (const auto& p : path) {
        if (p.first >= 0 && p.first < height && p.second >= 0 && p.second < width) {
            visualGrid[p.first][p.second] = '*';
        }
    }
    
    // Mark start and goal
    visualGrid[startX][startY] = 'S';
    visualGrid[goalX][goalY] = 'G';
    
    // Write to file
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            file << visualGrid[i][j] << \" \";
        }
        file << \"\n\";
    }
    
    file.close();
    std::cout << \"Grid with path exported to: \" << filename << std::endl;
}

#endif // UTILS_H
