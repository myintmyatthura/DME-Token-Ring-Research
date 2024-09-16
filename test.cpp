#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    int local_sum = 0;
    int global_sum = 0;
    
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Each process calculates its part of the sum
    local_sum = rank + 1; // Example: process rank is used to contribute to the sum

    // Reduce the sums from all processes to the master process (rank 0)
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Master process prints the result
    if (rank == 0) {
        std::cout << "The total sum is: " << global_sum << std::endl;
    }

    // Finalize the MPI environment
    MPI_Finalize();

    return 0;
}
