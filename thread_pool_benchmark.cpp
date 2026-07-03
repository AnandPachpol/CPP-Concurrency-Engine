#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <iomanip>

// =============================================================================
// THE ENGINE: SimpleThreadPool
// =============================================================================
class SimpleThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;

public:
    explicit SimpleThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                //while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return; 
                        task = std::move(tasks.front());
                        tasks.pop();
                    } 
                    task(); 
                //}
            });
        }
    }

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(std::move(task));
        }
        cv.notify_one(); 
    }

    ~SimpleThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all(); 
        for (std::thread &worker : workers) {
            if (worker.joinable()) worker.join(); 
        }
    }
};

// =============================================================================
// THE WORKLOAD: Sieve of Eratosthenes
// =============================================================================
long long countPrimes(uint32_t limit) {
    if (limit < 2) return 0;
    std::vector<bool> is_composite(limit + 1, false);

    for (uint32_t p = 2; (uint64_t)p * p <= limit; ++p) {
        if (!is_composite[p]) {
            for (uint64_t j = (uint64_t)p * p; j <= limit; j += p) {
                is_composite[j] = true;
            }
        }
    }

    long long count = 0;
    for (uint32_t i = 2; i <= limit; ++i) {
        if (!is_composite[i]) ++count;
    }
    return count;
}

// =============================================================================
// MAIN EXECUTION
// =============================================================================
int main() {
    constexpr uint32_t SIEVE_LIMIT = 5000000;
    constexpr int NUM_TASKS = 8;
    
    std::cout << "========================================\n";
    std::cout << "  BENCHMARK: CONCURRENT VS SEQUENTIAL\n";
    std::cout << "========================================\n";
    std::cout << "Workload: Sieve up to " << SIEVE_LIMIT << " (" << NUM_TASKS << " times)\n\n";

    // -------------------------------------------------------------------------
    // MODE 1: NORMAL / SEQUENTIAL MODE
    // -------------------------------------------------------------------------
    std::cout << "Running Normal Mode (Sequential)..." << std::endl;
    std::vector<long long> seqResults;
    seqResults.reserve(NUM_TASKS);

    auto seqStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_TASKS; ++i) {
        seqResults.push_back(countPrimes(SIEVE_LIMIT));
    }
    auto seqEnd = std::chrono::high_resolution_clock::now();
    auto seqMs = std::chrono::duration_cast<std::chrono::milliseconds>(seqEnd - seqStart).count();
    
    std::cout << "  Normal Mode Finished! Time: " << seqMs << " ms\n\n";

    // -------------------------------------------------------------------------
    // MODE 2: MULTI-THREADED / POOL MODE
    // -------------------------------------------------------------------------
    std::cout << "Running Thread Pool Mode (8 Workers)..." << std::endl;
    std::vector<long long> parResults(NUM_TASKS, 0);

    auto parStart = std::chrono::high_resolution_clock::now();
    {
        SimpleThreadPool pool(8);
        for (int i = 0; i < NUM_TASKS; ++i) {
            pool.enqueue([i, &parResults, SIEVE_LIMIT]() {
                parResults[i] = countPrimes(SIEVE_LIMIT);
            });
        }
    } // Destructor blocks until all 8 workers finish
    auto parEnd = std::chrono::high_resolution_clock::now();
    auto parMs = std::chrono::duration_cast<std::chrono::milliseconds>(parEnd - parStart).count();

    std::cout << "  Thread Pool Mode Finished! Time: " << parMs << " ms\n\n";

    // -------------------------------------------------------------------------
    // FINAL RESULTS & COMPARISON
    // -------------------------------------------------------------------------
    double speedup = static_cast<double>(seqMs) / static_cast<double>(parMs);

    std::cout << "========================================\n";
    std::cout << "  SUMMARY COMPARISON\n";
    std::cout << "========================================\n";
    std::cout << "  Normal Mode Execution Time : " << seqMs << " ms\n";
    std::cout << "  Thread Pool Execution Time : " << parMs << " ms\n";
    std::cout << "  Calculated Speedup Factor  : " << std::fixed << std::setprecision(2) << speedup << "x\n";
    
    // Check for correctness
    bool pass = true;
    for(int i = 0; i < NUM_TASKS; ++i) {
        if(seqResults[i] != parResults[i]) pass = false;
    }
    std::cout << "  Data Verification          : " << (pass ? "PASS" : "FAIL (Data Race)") << "\n";
    std::cout << "========================================\n";

    return 0;
}
