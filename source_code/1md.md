I am implementing a High Performance Computing project titled:

"High-Performance Parallel Pathfinding on Large-Scale Grids Using OpenMP and MPI".

The project goal is to implement a shortest path algorithm on very large grid maps and compare performance between serial, shared-memory, distributed-memory, and hybrid parallel implementations.

The implementation language must be C++ and run on Linux (Ubuntu wsl).

The project must include four versions:

1. Serial implementation
2. OpenMP shared-memory parallel implementation
3. MPI distributed-memory implementation
4. Hybrid MPI + OpenMP implementation

The project will simulate large grid environments with obstacles and compute the shortest path between a start node and a goal node.

------------------------------------------------

PROJECT ARCHITECTURE

The project should follow this folder structure:

project/
│
├── serial/
│   └── astar_serial.cpp
│
├── openmp/
│   └── astar_openmp.cpp
│
├── mpi/
│   └── astar_mpi.cpp
│
├── hybrid/
│   └── astar_hybrid.cpp
│
├── common/
│   ├── grid.h
│   ├── grid.cpp
│   ├── node.h
│   └── utils.h
│
└── results/
    └── performance_logs.txt

------------------------------------------------

PROJECT DESCRIPTION

The environment is a 2D grid representing a map.

Each cell can be:

0 = free space
1 = obstacle

Example grid:

S . . X .
. X . X .
. . . . G

Where:
S = start
G = goal
X = obstacle

The algorithm should compute the shortest path avoiding obstacles.

The grid size should be configurable and support very large grids like:

1000 x 1000
5000 x 5000
10000 x 10000

------------------------------------------------

ALGORITHM

Use the A* (A-star) pathfinding algorithm.

Each node must store:

- x coordinate
- y coordinate
- g cost (distance from start)
- h cost (heuristic distance to goal)
- f cost (g + h)
- parent pointer

Use Manhattan distance as the heuristic:

h(n) = |x1 - x2| + |y1 - y2|

Use a priority queue to manage the open set.

------------------------------------------------

IMPLEMENTATION STEPS

Step 1 — Grid Generation

Create a function that generates a grid with random obstacles.

Input parameters:
grid width
grid height
obstacle density (for example 30%)

Output:
2D vector grid

Example:

vector<vector<int>> grid;

------------------------------------------------

Step 2 — Serial A* Implementation

Implement the standard A* algorithm.

Requirements:

- open list using priority_queue
- closed list to mark visited nodes
- 4-direction movement (up, down, left, right)
- reconstruct final path

The program should print:

Path length
Execution time

Measure execution time using:

std::chrono

------------------------------------------------

Step 3 — OpenMP Parallel Version

Parallelize computational parts using OpenMP.

Possible areas to parallelize:

1. Neighbor exploration
2. Heuristic calculations
3. Node expansion

Use OpenMP directives such as:

#pragma omp parallel
#pragma omp for
#pragma omp critical

Allow the number of threads to be configurable.

Example:

OMP_NUM_THREADS=4

Measure execution time and compare with serial version.

------------------------------------------------

Step 4 — MPI Distributed Version

Partition the grid across multiple processes.

Each MPI process should handle a section of the grid.

Example partition:

Process 0 → top rows
Process 1 → middle rows
Process 2 → bottom rows

Processes must communicate when the path crosses region boundaries.

Use MPI functions such as:

MPI_Init
MPI_Comm_rank
MPI_Comm_size
MPI_Send
MPI_Recv
MPI_Barrier
MPI_Finalize

Collect results from all processes.

------------------------------------------------

Step 5 — Hybrid MPI + OpenMP

Combine distributed and shared memory parallelism.

Each MPI process handles a grid partition.

Inside each process:

OpenMP threads parallelize local node expansions.

------------------------------------------------

PERFORMANCE EVALUATION

The program must record:

Execution time
Number of threads
Number of processes
Grid size

Compute metrics:

Speedup = SerialTime / ParallelTime

Efficiency = Speedup / NumberOfThreads

------------------------------------------------

EXPERIMENTS

Run experiments using:

Grid sizes:

1000x1000
2000x2000
5000x5000

Thread counts:

1
2
4
8

Process counts:

1
2
4

Save results in a file for plotting graphs later.

------------------------------------------------

OUTPUT FORMAT

Each program should print:

Grid size
Threads/processes used
Execution time
Path length

------------------------------------------------

ADDITIONAL FEATURES (OPTIONAL)

Add a function to export the grid and path to a file so that it can be visualized later.

------------------------------------------------

Now help me implement this project step by step starting with:

1. Grid data structures
2. Node structure
3. Serial A* implementation
4. Performance timer