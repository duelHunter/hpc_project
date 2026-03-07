# High-Performance Parallel Pathfinding on Large-Scale Grids

This project implements A* pathfinding algorithm with four different approaches:
1. Serial implementation
2. OpenMP shared-memory parallel implementation
3. MPI distributed-memory implementation
4. Hybrid MPI + OpenMP implementation

## Project Structure

```
source_code/
│
├── serial/
│   └── astar_serial.cpp       # Serial A* implementation
│
├── openmp/
│   └── astar_openmp.cpp       # OpenMP parallel version (TBD)
│
├── mpi/
│   └── astar_mpi.cpp          # MPI distributed version (TBD)
│
├── hybrid/
│   └── astar_hybrid.cpp       # Hybrid MPI+OpenMP version (TBD)
│
├── common/
│   ├── grid.h                 # Grid class header
│   ├── grid.cpp               # Grid class implementation
│   ├── node.h                 # Node structure for A*
│   └── utils.h                # Utility functions and timer
│
├── results/
│   └── performance_logs.txt   # Performance results
│
├── Makefile                   # Build system
└── README.md                  # This file
```

## Requirements

- **Compiler**: g++ with C++11 support
- **OpenMP**: For shared-memory parallelism
- **MPI**: For distributed-memory parallelism (OpenMPI or MPICH)
- **OS**: Linux (Ubuntu WSL recommended for Windows)

## Installation on Ubuntu WSL

```bash
# Install g++ compiler
sudo apt update
sudo apt install build-essential

# Install OpenMP (usually comes with g++)
sudo apt install libomp-dev

# Install MPI
sudo apt install mpich
```

## Compilation

### Serial Version
```bash
# Build the serial version
make clean
make serial_astar
```

### OpenMP Version
```bash
make openmp_astar
```

### MPI Version
```bash
make mpi_astar
```

### Hybrid Version
```bash
make hybrid_astar
```

### Build All
```bash
make all
```

## Execution

### Serial Version

```bash
# Run with default parameters (1000x1000 grid, 30% obstacles)
./serial/astar_serial

# Run with custom parameters
./serial/astar_serial <width> <height> <obstacle_density> <seed>

# Examples:
./serial/astar_serial 1000 1000 0.3 42
./serial/astar_serial 2000 2000 0.25 100
./serial/astar_serial 5000 5000 0.30 200
```

### OpenMP Version

```bash
# Set number of threads
export OMP_NUM_THREADS=4

# Run
./openmp/astar_openmp 1000 1000 0.3 42

# Test with different thread counts
for threads in 1 2 4 8; do
    export OMP_NUM_THREADS=$threads
    ./openmp/astar_openmp 2000 2000 0.3
done
```

### MPI Version

```bash
# Run with 4 processes
mpirun -np 4 ./mpi/astar_mpi 2000 2000 0.3 42

# Test with different process counts
for procs in 1 2 4 8; do
    mpirun -np $procs ./mpi/astar_mpi 2000 2000 0.3
done
```

### Hybrid Version

```bash
# Run with 2 processes and 4 threads per process
export OMP_NUM_THREADS=4
mpirun -np 2 ./hybrid/astar_hybrid 2000 2000 0.3 42
```

## Performance Testing

Run comprehensive tests with different configurations:

```bash
# Test script example
#!/bin/bash

GRIDS=(\"1000 1000\" \"2000 2000\" \"5000 5000\")
THREADS=(1 2 4 8)
PROCESSES=(1 2 4)

# Serial tests
for grid in \"${GRIDS[@]}\"; do
    ./serial/astar_serial $grid 0.3
done

# OpenMP tests
for grid in \"${GRIDS[@]}\"; do
    for t in \"${THREADS[@]}\"; do
        export OMP_NUM_THREADS=$t
        ./openmp/astar_openmp $grid 0.3
    done
done

# MPI tests
for grid in \"${GRIDS[@]}\"; do
    for p in \"${PROCESSES[@]}\"; do
        mpirun -np $p ./mpi/astar_mpi $grid 0.3
    done
done
```

## Output Format

Each run produces output like:

```
========================================
Serial A* Pathfinding
========================================
Grid size: 1000x1000
Obstacle density: 30%
========================================
Generated 1000x1000 grid with 30% obstacles
Start: (0, 0)
Goal: (999, 999)
========================================
Nodes expanded: 15234
========================================
Path found!
Path length: 1998 steps
Path found with 1998 steps:
(0, 0) -> (0, 1) -> (0, 2) -> ...
Execution time: 245.67 ms
========================================
```

## Results File

Performance results are automatically saved to esults/performance_logs.txt:

```
version,grid_size,threads,processes,execution_time,path_length
serial,1000x1000,1,1,245.67,1998
openmp,1000x1000,4,1,89.32,1998
mpi,1000x1000,1,4,156.45,1998
hybrid,1000x1000,4,4,67.89,1998
```

## Visualization

For small grids (≤100x100), the program exports a visualization file showing the path:

```
results/serial_path.txt
```

Symbols:
- . = free space
- X = obstacle
- * = path
- S = start
- G = goal

## Performance Metrics

Calculate speedup and efficiency:

```
Speedup = Serial Time / Parallel Time
Efficiency = Speedup / Number of Processors
```

## Current Status

✅ Serial implementation - COMPLETE
⏳ OpenMP implementation - TODO
⏳ MPI implementation - TODO
⏳ Hybrid implementation - TODO

## Next Steps

1. Verify serial implementation works correctly
2. Implement OpenMP parallel version
3. Implement MPI distributed version
4. Implement hybrid version
5. Run comprehensive performance tests
6. Analyze results and create plots
