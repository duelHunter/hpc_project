# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3

# OpenMP flags
OMPFLAGS = -fopenmp

# MPI compiler
MPICXX = mpic++

# Directories
COMMON = common
SERIAL = serial
OPENMP = openmp
MPI = mpi
HYBRID = hybrid
RESULTS = results

# Common object files
COMMON_OBJ = $(COMMON)/grid.o

# Targets
all: serial_astar

# Compile common files
$(COMMON)/grid.o: $(COMMON)/grid.cpp $(COMMON)/grid.h
$(CXX) $(CXXFLAGS) -c $< -o $@

# Serial version
serial_astar: $(SERIAL)/astar_serial.cpp $(COMMON_OBJ)
$(CXX) $(CXXFLAGS) $^ -o $(SERIAL)/astar_serial

# OpenMP version (to be implemented)
openmp_astar: $(OPENMP)/astar_openmp.cpp $(COMMON_OBJ)
$(CXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o $(OPENMP)/astar_openmp

# MPI version (to be implemented)
mpi_astar: $(MPI)/astar_mpi.cpp $(COMMON_OBJ)
$(MPICXX) $(CXXFLAGS) $^ -o $(MPI)/astar_mpi

# Hybrid version (to be implemented)
hybrid_astar: $(HYBRID)/astar_hybrid.cpp $(COMMON_OBJ)
$(MPICXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o $(HYBRID)/astar_hybrid

# Clean
clean:
rm -f $(COMMON)/*.o
rm -f $(SERIAL)/astar_serial
rm -f $(OPENMP)/astar_openmp
rm -f $(MPI)/astar_mpi
rm -f $(HYBRID)/astar_hybrid

# Run serial test
test_serial: serial_astar
./$(SERIAL)/astar_serial 1000 1000 0.3

.PHONY: all clean test_serial
