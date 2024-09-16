#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

typedef struct {
    int timestamp;
    int rank;
} Request;

// function to compare the time stamps or ranks 
int compare_requests(const void *a, const void *b) {
    Request *reqA = (Request *) a;
    Request *reqB = (Request *) b;
    if (reqA->timestamp == reqB->timestamp) return reqA->rank - reqB->rank;
    return reqA->timestamp - reqB->timestamp;
}

int main(int argc, char **argv) {
    // initializations 
    int world_size, world_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    char filename[20];
    sprintf(filename, "output_%d.txt", world_rank);
    freopen(filename, "w", stdout);

    MPI_Datatype MPI_REQUEST;
    MPI_Type_contiguous(2, MPI_INT, &MPI_REQUEST);
    MPI_Type_commit(&MPI_REQUEST);

    bool requesting = false;
    srand(time(NULL) + world_rank);
    int clock = rand() % 10;
    Request request_queue[world_size];
    int request_queue_size = 0;
    int replies_needed = world_size - 1;
    int replies_received = 0;

    // Start of the algorithm
    for (int i = 0; i < 2; i++) {
        clock++;
        requesting = true;
        Request my_request = {clock, world_rank};
        request_queue[request_queue_size++] = my_request;
        // qsort to sort the queue array appropriately
        qsort(request_queue, request_queue_size, sizeof(Request), compare_requests);

        // Send request to all other processes
        printf("Process %d sending requests at time %d\n", world_rank, clock);
        fflush(stdout);
        for (int j = 0; j < world_size; j++) {
            if (j != world_rank) {
                MPI_Send(&my_request, 1, MPI_REQUEST, j, REQUEST_TAG, MPI_COMM_WORLD);
                printf("Process %d sent request to Process %d\n", world_rank, j);
                fflush(stdout);
            }
        }

        // Wait for replies from all other processes
        while (replies_received < replies_needed) {
            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int source = status.MPI_SOURCE;
            int tag = status.MPI_TAG;

            // if it is a request tag, we update the request queue and send a reply tag
            if (tag == REQUEST_TAG) {
                Request incoming_request;
                MPI_Recv(&incoming_request, 1, MPI_REQUEST, source, REQUEST_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Process %d received request from Process %d at time [ %d ]\n", world_rank, source, clock);
                fflush(stdout);
                clock = (incoming_request.timestamp > clock ? incoming_request.timestamp : clock) + 1;
                request_queue[request_queue_size++] = incoming_request;
                qsort(request_queue, request_queue_size, sizeof(Request), compare_requests);
                MPI_Send(NULL, 0, MPI_INT, source, REPLY_TAG, MPI_COMM_WORLD);
                printf("Process %d sent reply to Process %d at time         [ %d ] \n", world_rank, source, clock);
                fflush(stdout);
            // if it is a reply tag, we receive it and increment while loop
            } else if (tag == REPLY_TAG) {
                MPI_Recv(NULL, 0, MPI_INT, source, REPLY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Process %d received reply from Process %d at time   [ %d ]\n", world_rank, source, clock);
                fflush(stdout);
                replies_received++;
            // if it is a release tag, we update the request queue as necessary and break immedietly 
            } else if (tag == RELEASE_TAG) {
                MPI_Recv(NULL, 0, MPI_INT, source, RELEASE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Process %d received release from Process %d\n", world_rank, source);
                fflush(stdout);
                for (int k = 0; k < request_queue_size; k++) {
                    if (request_queue[k].rank == source) {
                        for (int l = k; l < request_queue_size - 1; l++) {
                            request_queue[l] = request_queue[l + 1];
                        }
                        request_queue_size--;
                        break;
                    }
                }
            }
        }

        // Enter critical section
        printf("Process %d entering critical section at time       [ %d ] \n", world_rank, clock);
        fflush(stdout);
        sleep(2);  // Sleep for 2 seconds
        printf("Process %d exiting critical section at time        [ %d ]\n", world_rank, clock);
        fflush(stdout);

        MPI_Barrier(MPI_COMM_WORLD); // Ensures that no process enters the critical section before all processes have exited it

        // Exit critical section
        requesting = false;
        replies_received = 0;

        // Release critical section and send reply to pending requests
        for (int j = 1; j < request_queue_size; j++) {
            if (request_queue[j].rank != world_rank) {
                MPI_Send(NULL, 0, MPI_INT, request_queue[j].rank, RELEASE_TAG, MPI_COMM_WORLD);
        printf("Process %d sent release to Process %d at time       [ %d ]\n", world_rank, request_queue[j].rank, clock);
                fflush(stdout);
            }
        }
        request_queue_size = 0;

        // Print separator
        printf("------------------------------------------------------------------------------------------\n");
        fflush(stdout);
    }

    fclose(stdout);
    MPI_Finalize();
    return 0;
}
