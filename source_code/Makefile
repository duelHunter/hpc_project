CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3
OMPFLAGS = -fopenmp
MPICXX = mpic++
COMMON_OBJ = common/grid.o

all: serial_astar serial_jps openmp_jps mpi_jps hybrid_jps

common/grid.o: common/grid.cpp common/grid.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

serial_astar: serial/astar_serial.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o serial/astar_serial

serial_jps: serial/jps_serial.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o serial/jps_serial

openmp_astar: openmp/astar_openmp.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o openmp/astar_openmp

openmp_jps: openmp/jps_openmp.cpp $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o openmp/jps_openmp

mpi_astar: mpi/astar_mpi.cpp $(COMMON_OBJ)
	$(MPICXX) $(CXXFLAGS) $^ -o mpi/astar_mpi

mpi_jps: mpi/jps_mpi.cpp $(COMMON_OBJ)
	$(MPICXX) $(CXXFLAGS) $^ -o mpi/jps_mpi


hybrid_astar: hybrid/astar_hybrid.cpp $(COMMON_OBJ)
	$(MPICXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o hybrid/astar_hybrid

hybrid_jps: hybrid/jps_hybrid.cpp $(COMMON_OBJ)
	$(MPICXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o hybrid/jps_hybrid

clean:
	rm -f mpi/jps_mpi hybrid/jps_hybrid common/grid.o serial/astar_serial serial/jps_serial openmp/astar_openmp openmp/jps_openmp mpi/astar_mpi hybrid/astar_hybrid


.PHONY: all clean
