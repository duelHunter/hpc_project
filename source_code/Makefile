CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3
OMPFLAGS = -fopenmp
MPICXX = mpic++
COMMON_OBJ = common/grid.o

all: serial_astar

common/grid.o: common/grid.cpp common/grid.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

serial_astar: serial/astar_serial.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o serial/astar_serial

openmp_astar: openmp/astar_openmp.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o openmp/astar_openmp

mpi_astar: mpi/astar_mpi.cpp $(COMMON_OBJ)
	$(MPICXX) $(CXXFLAGS) $^ -o mpi/astar_mpi

hybrid_astar: hybrid/astar_hybrid.cpp $(COMMON_OBJ)
	$(MPICXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o hybrid/astar_hybrid

clean:
	rm -f common/grid.o serial/astar_serial openmp/astar_openmp mpi/astar_mpi hybrid/astar_hybrid

.PHONY: all clean
