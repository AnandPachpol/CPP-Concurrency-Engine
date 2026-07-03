# Multithreaded C++ Performance Engine

A C++17 project demonstrating two fundamental approaches to parallel programming:

- **Task-Level Parallelism** using a custom producer-consumer thread pool
- **Data-Level Parallelism** using a parallel segmented Sieve of Eratosthenes

The project benchmarks both implementations against sequential baselines to evaluate scalability, correctness, and real-world speedup.

---

## Overview

Modern multicore processors provide significant computational power, but applications must be designed to utilize multiple cores efficiently.

This project explores two different parallelization strategies:

1. **Task Parallelism**
   - Multiple independent tasks execute concurrently using a custom thread pool.

2. **Data Parallelism**
   - A large computational problem is divided into independent data segments that are processed simultaneously by multiple threads.

Both implementations are validated against sequential versions to ensure correctness before benchmarking.

---

## Features

### Task-Level Parallelism

- Custom producer-consumer thread pool
- Implemented using C++17 threading primitives
- Uses:
  - `std::thread`
  - `std::mutex`
  - `std::condition_variable`
  - `std::unique_lock`
  - `std::lock_guard`
- RAII-based mutex management
- Thread-safe task queue
- Worker thread synchronization
- Automatic cleanup through destructor

The workload consists of multiple independent executions of the Sieve of Eratosthenes, making it ideal for demonstrating task parallelism.

---

### Data-Level Parallelism

Implements a parallel segmented Sieve of Eratosthenes.

Features include:

- Sequential preprocessing of primes up to √N
- Offset-indexed local sieve segments
- Independent work assigned to each thread
- Lock-free computation during sieving
- Thread-local memory for composite marking
- Reduction of partial prime counts into the final result

Each thread processes a unique portion of the search space, eliminating shared write conflicts.

---

## Project Structure

```
.
├── main2.cpp      # Task-level parallelism (Thread Pool)
├── real.cpp       # Data-level parallel segmented sieve
└── README.md
```

---

## Benchmarks

Each implementation is compared against its sequential counterpart.

Metrics collected include:

- Execution time
- Speedup
- Correctness verification
- Data race validation

Typical observations on an 8-core machine:

- Approximately **5× speedup**
- Correct output matching the sequential implementation
- No data races observed

Actual performance depends on processor architecture and compiler optimizations.

---

## Technologies Used

- C++17
- STL Threads
- Mutexes
- Condition Variables
- RAII Locking
- High Resolution Clock (`std::chrono`)
- Parallel Algorithms
- Producer-Consumer Synchronization

---

## Building

Compile using any C++17 compatible compiler.

### Thread Pool Benchmark

```bash
g++ -std=c++17 -pthread main2.cpp -O2 -o thread_pool
./thread_pool
```

### Parallel Segmented Sieve

```bash
g++ -std=c++17 -pthread real.cpp -O2 -o segmented_sieve
./segmented_sieve
```

---

## Implementation Highlights

### Thread Pool

- Fixed-size worker pool
- Thread-safe task queue
- Producer-consumer synchronization
- Condition-variable based sleeping
- Automatic thread joining
- Minimal synchronization overhead

---

### Parallel Segmented Sieve

- Precomputes base primes sequentially
- Divides the search interval among threads
- Thread-local marking arrays
- Shared read-only prime list
- Parallel reduction of results

---

## Learning Outcomes

This project provided hands-on experience with:

- Concurrent programming in C++17
- Thread synchronization
- Mutex locking strategies
- Condition variables
- RAII for safe lock management
- Producer-consumer architecture
- Data partitioning for parallel algorithms
- Performance benchmarking
- Scalability analysis
- Race-condition avoidance

---

## Future Improvements

- Dynamic work stealing scheduler
- Lock-free task queue
- Generic reusable thread pool API
- Configurable scheduling policies
- NUMA-aware partitioning
- Performance visualization
- Additional benchmark workloads

---

## Author

**Anand Pachpol**
