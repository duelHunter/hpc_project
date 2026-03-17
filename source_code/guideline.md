# High-Performance Parallel Pathfinding Project (A* → JPS → OpenMP → MPI)

## 🎯 Goal

Implement and optimize grid-based pathfinding for large-scale environments (up to 10000×10000) using:

1. Serial A* (baseline)
2. Serial JPS (optimized)
3. OpenMP-based parallel JPS
4. (Optional) MPI distributed version

The focus is on reducing execution time and analyzing performance.

---

## 📌 Existing Context

* Language: C++
* Grid class already exists (`grid.h`)
* Utility functions exist (`utils.h`)
* A working serial A* implementation is available
* Grid uses:

  * `grid.isValid(x, y)` → checks bounds + obstacles
  * `DX[]`, `DY[]` → direction vectors (4 or 8 directions)

---

## ⚠️ Important Constraints

* Grid size can be very large → must optimize memory access
* Cost between nodes = 1 (uniform grid)
* Must maintain correctness (valid shortest path unless stated otherwise)
* Avoid race conditions in parallel versions

---

# ✅ STEP 1 — SERIAL A* (BASELINE)

## Requirements:

* Use priority queue (`std::priority_queue`)
* Maintain:

  * `gScore` (distance from start)
  * `parent` (path reconstruction)
  * `closed` (visited nodes)
* Heuristic: Manhattan distance

## Output:

* Path (vector of `(x, y)`)
* Execution time
* Nodes expanded

---

# 🚀 STEP 2 — SERIAL JPS (Jump Point Search)

## Objective:

Replace neighbor expansion with jump-based expansion.

## Key Tasks:

### 1. Implement `jump()` function:

```cpp
int jump(int x, int y, int dx, int dy);
```

### Behavior:

* Move in direction `(dx, dy)`
* Stop when:

  * Out of bounds or obstacle → return -1
  * Goal reached → return index
  * Forced neighbor detected → return index

---

### 2. Detect forced neighbors:

* Straight movement:

  * Check side-blocked cells that force turning
* Diagonal movement:

  * Check both horizontal and vertical forced conditions

---

### 3. Replace A* expansion:

Instead of:

```cpp
for each neighbor
```

Use:

```cpp
for each direction:
    jump(...)
```

---

### 4. Update cost:

Instead of:

```cpp
ng = cg + 1;
```

Use:

```cpp
ng = cg + distance between current and jump point
```

---

## Expected Result:

* Much fewer nodes expanded
* Significant speedup over A*

---

# ⚡ STEP 3 — OPENMP PARALLEL JPS

## Objective:

Parallelize node expansion safely.

## DO NOT:

* Modify shared structures directly inside parallel loop

## Use this pattern:

```cpp
#pragma omp parallel
{
    vector<local results>

    #pragma omp for schedule(dynamic,1)
    for(batch of nodes){
        compute jump points
        store results locally
    }

    #pragma omp critical
    {
        merge into:
        - gScore
        - parent
        - openList
    }
}
```

---

## Important:

* Do NOT parallelize small loops (like directions)
* Parallelize across multiple nodes (batch processing)
* Minimize time inside `critical`

---

## Parameters to tune:

* Batch size (important for performance)
* Number of threads
* Scheduling strategy

---

# 🌐 STEP 4 — MPI VERSION (OPTIONAL)

## Objective:

Distribute grid across multiple processes.

## Strategy:

* Divide grid into regions (domain decomposition)
* Each process runs local JPS
* Communicate boundary nodes using MPI

## Required MPI Functions:

* `MPI_Init`
* `MPI_Send`
* `MPI_Recv`
* `MPI_Barrier`
* `MPI_Finalize`

---

## Key Challenge:

* Synchronizing frontier nodes across processes
* Avoid excessive communication

---

# 📊 PERFORMANCE EVALUATION

## Metrics to measure:

* Execution time (ms)
* Nodes expanded
* Speedup:

```text
Speedup = Serial Time / Parallel Time
```

* Efficiency:

```text
Efficiency = Speedup / Number of Threads
```

---

## Experiments:

* Grid sizes: 1000×1000 → 10000×10000
* Threads: 1, 2, 4, 8, ...
* Compare:

  * A* vs JPS
  * JPS vs OpenMP JPS

---

# 🧠 EXPECTED INSIGHTS

* A* does not parallelize well due to priority queue bottleneck
* JPS reduces node expansions significantly
* OpenMP improves performance only when workload is large enough
* Memory access patterns impact performance heavily

---

# 📁 OUTPUT FILES

Save results to:

```
results/performance_logs.txt
```

Format:

```
algorithm,grid_size,threads,time_ms,path_length
```

---

# 🧪 DEBUGGING TIPS

* First ensure JPS correctness before parallelizing
* Compare paths with A*
* Check for:

  * Infinite recursion in `jump()`
  * Incorrect forced neighbor detection
  * Race conditions in OpenMP

---

# 🎯 FINAL GOAL

Produce:

* Faster pathfinding than A*
* Scalable parallel version
* Clear performance comparison

---

Generate clean, modular, well-documented C++ code for all steps.
