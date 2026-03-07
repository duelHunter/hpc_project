#ifndef GRID_H
#define GRID_H

#include <vector>
#include <random>
#include <iostream>

/**
 * Grid class for managing the 2D environment
 */
class Grid {
private:
    int width;
    int height;
    std::vector<std::vector<int>> data;
    
public:
    // Constructor
    Grid(int w, int h);
    
    // Generate random obstacles
    void generateObstacles(double density, unsigned int seed = 42);
    
    // Clear specific path (ensure start and goal are free)
    void clearCell(int x, int y);
    
    // Check if cell is valid and free
    bool isValid(int x, int y) const;
    
    // Check if cell is obstacle
    bool isObstacle(int x, int y) const;
    
    // Get grid dimensions
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Get the grid data
    const std::vector<std::vector<int>>& getData() const { return data; }
    
    // Set cell value
    void setCell(int x, int y, int value);
    
    // Get cell value
    int getCell(int x, int y) const;
    
    // Print grid (for debugging small grids)
    void print(int maxSize = 20) const;
};

#endif // GRID_H
