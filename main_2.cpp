#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unistd.h>

#define REQUEST_TAG 1
#define REPLY_TAG 2
#define RELEASE_TAG 3

/*
Name - Myint Myat Thura
ID - 31861067

Instructions :

Just run the makefile by typing "make" in the command line.
You may read the output by finding the process with the lowest start time, and you may see
that the code does indeed work. The proof of correctness is done by splitting up the process printing 
into seperate files. Please refer to the recording for more details.

MAKE FILE CODE :

CC = mpicc
CFLAGS =
LDFLAGS = -lm
EXECUTABLE = Output
NP = 4

.PHONY: all run clean

all: $(EXECUTABLE) run

$(EXECUTABLE): main.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: $(EXECUTABLE)
	@echo "Number of processes: 4"
	@echo "Logical Clock Start Time : Random"
	@echo "Sleep: 2s"
	mpirun -oversubscribe -np $(NP) ./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

    
*/

struct Request {
    int timestamp;
    int rank;
};

// Function to compare timestamps or ranks
bool compare_requests(const Request &a, const Request &b) {
    if (a.timestamp == b.timestamp) return a.rank < b.rank;
    return a.timestamp < b.timestamp;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    std::ofstream outfile("output_" + std::to_string(world_rank) + ".txt");

    MPI_Datatype MPI_REQUEST;
    MPI_Type_contiguous(2, MPI_INT, &MPI_REQUEST);
    MPI_Type_commit(&MPI_REQUEST);

    bool requesting = false;
    srand(time(NULL) + world_rank);
    int clock = rand() % 10;
    std::vector<Request> request_queue;
    int replies_needed = world_size - 1;
    int replies_received = 0;

    // Start of the algorithm
    for (int i = 0; i < 2; ++i) {
        ++clock;
        requesting = true;
        Request my_request = {clock, world_rank};
        request_queue.push_back(my_request);
        std::sort(request_queue.begin(), request_queue.end(), compare_requests);

        // Send request to all other processes
        outfile << "Process " << world_rank << " sending requests at time " << clock << std::endl;
        outfile.flush();
        for (int j = 0; j < world_size; ++j) {
            if (j != world_rank) {
                MPI_Send(&my_request, 1, MPI_REQUEST, j, REQUEST_TAG, MPI_COMM_WORLD);
                outfile << "Process " << world_rank << " sent request to Process " << j << std::endl;
                outfile.flush();
            }
        }

        // Wait for replies from all other processes
        while (replies_received < replies_needed) {
            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int source = status.MPI_SOURCE;
            int tag = status.MPI_TAG;

            if (tag == REQUEST_TAG) {
                Request incoming_request;
                MPI_Recv(&incoming_request, 1, MPI_REQUEST, source, REQUEST_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                outfile << "Process " << world_rank << " received request from Process " << source << " at time [ " << clock << " ]" << std::endl;
                outfile.flush();
                clock = std::max(incoming_request.timestamp, clock) + 1;
                request_queue.push_back(incoming_request);
                std::sort(request_queue.begin(), request_queue.end(), compare_requests);
                MPI_Send(nullptr, 0, MPI_INT, source, REPLY_TAG, MPI_COMM_WORLD);
                outfile << "Process " << world_rank << " sent reply to Process " << source << " at time [ " << clock << " ]" << std::endl;
                outfile.flush();
            } else if (tag == REPLY_TAG) {
                MPI_Recv(nullptr, 0, MPI_INT, source, REPLY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                outfile << "Process " << world_rank << " received reply from Process " << source << " at time [ " << clock << " ]" << std::endl;
                outfile.flush();
                ++replies_received;
            } else if (tag == RELEASE_TAG) {
                MPI_Recv(nullptr, 0, MPI_INT, source, RELEASE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                outfile << "Process " << world_rank << " received release from Process " << source << std::endl;
                outfile.flush();
                auto it = std::remove_if(request_queue.begin(), request_queue.end(),
                                         [source](const Request &r) { return r.rank == source; });
                request_queue.erase(it, request_queue.end());
            }
        }

        // Enter critical section
        outfile << "Process " << world_rank << " entering critical section at time [ " << clock << " ]" << std::endl;
        outfile.flush();
        sleep(2);  // Sleep for 2 seconds
        outfile << "Process " << world_rank << " exiting critical section at time [ " << clock << " ]" << std::endl;
        outfile.flush();

        MPI_Barrier(MPI_COMM_WORLD); // Ensures that no process enters the critical section before all processes have exited it

        // Exit critical section
        requesting = false;
        replies_received = 0;

        // Release critical section and send reply to pending requests
        for (size_t j = 1; j < request_queue.size(); ++j) {
            if (request_queue[j].rank != world_rank) {
                MPI_Send(nullptr, 0, MPI_INT, request_queue[j].rank, RELEASE_TAG, MPI_COMM_WORLD);
                outfile << "Process " << world_rank << " sent release to Process " << request_queue[j].rank << " at time [ " << clock << " ]" << std::endl;
                outfile.flush();
            }
        }
        request_queue.clear();

        // Print separator
        outfile << "------------------------------------------------------------------------------------------" << std::endl;
        outfile.flush();
    }

    outfile.close();
    MPI_Finalize();
    return 0;
}
