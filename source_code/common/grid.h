#ifndef GRID_H
#define GRID_H

#include <vector>
#include <random>
#include <iostream>

class Grid {
private:
    int width;
    int height;
    std::vector<std::vector<int>> data;

public:
    Grid(int w, int h);
    void generateObstacles(double density, unsigned int seed = 42);
    void clearCell(int x, int y);
    bool isValid(int x, int y) const;
    bool isObstacle(int x, int y) const;
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    const std::vector<std::vector<int>>& getData() const { return data; }
    void setCell(int x, int y, int value);
    int getCell(int x, int y) const;
    void print(int maxSize = 20) const;
};

#endif
