# Distributed Mutual Exclusion Algorithm Simulation with **MPI in C++**

---

## A simulation tool for Distributed Mutual Exclusion Algorithms using MPI in C++, with command-line support for distributed execution and visualization.

---

An innovative and scalable simulation tool developed in C++ utilizing MPI (Message Passing Interface) for distributed mutual exclusion algorithms. This project includes the implementation of a distributed mutual exclusion algorithm inspired by Leslie Lamport's research, featuring detailed proof of concept and supporting materials. The tool allows the simulation of multiple processes with real-time logging and analysis of request and reply handling for critical section access. Ideal for exploring distributed algorithms and visualizing their behavior in a parallel computing environment.

**Features:**

- Simulate distributed mutual exclusion algorithms using MPI
- Real-time logging of request, reply, and release events
- Visualization of critical section access and inter-process communication
- Command-line interface for distributed execution
- Detailed proof of concept with C++ implementation and associated presentation
- Includes a comprehensive presentation and video explaining the algorithm and its implementation

---

**Pre-requisites for execution:**

- MPI library (e.g., OpenMPI)
- C++ Standard Library
- Libraries: `<mpi.h>`, `<iostream>`, `<fstream>`, `<vector>`, `<algorithm>`, `<unistd.h>`

---

**How to use:**

Running this command will execute the simulation across multiple MPI processes, with logging to individual files for each process.

```
make
make run
```
