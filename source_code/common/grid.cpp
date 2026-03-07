#include \"grid.h\"

Grid::Grid(int w, int h) : width(w), height(h) {
    data.resize(height, std::vector<int>(width, 0));
}

void Grid::generateObstacles(double density, unsigned int seed) {
    if (density < 0.0 || density > 1.0) {
        std::cerr << \"Warning: Density should be between 0 and 1. Setting to 0.3\" << std::endl;
        density = 0.3;
    }
    
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (dist(rng) < density) {
                data[i][j] = 1;  // Obstacle
            } else {
                data[i][j] = 0;  // Free space
            }
        }
    }
    
    std::cout << \"Generated \" << width << \"x\" << height 
              << \" grid with \" << (density * 100) << \"% obstacles\" << std::endl;
}

void Grid::clearCell(int x, int y) {
    if (x >= 0 && x < height && y >= 0 && y < width) {
        data[x][y] = 0;
    }
}

bool Grid::isValid(int x, int y) const {
    return x >= 0 && x < height && y >= 0 && y < width && data[x][y] == 0;
}

bool Grid::isObstacle(int x, int y) const {
    if (x < 0 || x >= height || y < 0 || y >= width) {
        return true;  // Out of bounds treated as obstacle
    }
    return data[x][y] == 1;
}

void Grid::setCell(int x, int y, int value) {
    if (x >= 0 && x < height && y >= 0 && y < width) {
        data[x][y] = value;
    }
}

int Grid::getCell(int x, int y) const {
    if (x >= 0 && x < height && y >= 0 && y < width) {
        return data[x][y];
    }
    return -1;  // Invalid
}

void Grid::print(int maxSize) const {
    if (height > maxSize || width > maxSize) {
        std::cout << \"Grid too large to print (size: \" 
                  << width << \"x\" << height << \")\" << std::endl;
        return;
    }
    
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            std::cout << (data[i][j] == 1 ? 'X' : '.') << \" \";
        }
        std::cout << std::endl;
    }
}
