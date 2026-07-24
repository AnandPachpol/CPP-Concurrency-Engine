#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>

std::vector<uint32_t> smallPrimesUpTo(uint32_t limit) {
    std::vector<bool> is_composite(limit + 1, false);
    std::vector<uint32_t> primes;
    for (uint32_t p = 2; p <= limit; ++p) {
        if (!is_composite[p]) {
            primes.push_back(p);
            for (uint64_t j = (uint64_t)p * p; j <= limit; j += p)
                is_composite[j] = true;
        }
    }
    return primes;
}

void sieveSegment(uint64_t low, uint64_t high,
                   const std::vector<uint32_t>& smallPrimes,
                   long long& resultSlot) {
    uint64_t segSize = high - low + 1;
    std::vector<bool> is_composite(segSize, false); 

    for (uint32_t p : smallPrimes) {
        uint64_t start = std::max((uint64_t)p * p, ((low + p - 1) / p) * p);
        for (uint64_t j = start; j <= high; j += p) {
            is_composite[j - low] = true; 
        }
    }

    long long count = 0;
    for (uint64_t i = 0; i < segSize; ++i) {
        uint64_t number = low + i;
        if (number < 2) continue;
        if (!is_composite[i]) ++count;
    }
    resultSlot = count;
}

long long countPrimesSequential(uint64_t limit) {
    std::vector<bool> is_composite(limit + 1, false);
    for (uint64_t p = 2; p * p <= limit; ++p) {
        if (!is_composite[p]) {
            for (uint64_t j = p * p; j <= limit; j += p)
                is_composite[j] = true;
        }
    }
    long long count = 0;
    for (uint64_t i = 2; i <= limit; ++i)
        if (!is_composite[i]) ++count;
    return count;
}

int main() {
    constexpr uint64_t N = 100'000'000;
    unsigned numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;

    std::cout << "========================================\n";
    std::cout << "  PARALLEL SEGMENTED SIEVE (single task)\n";
    std::cout << "========================================\n";
    std::cout << "N = " << N << ", using " << numThreads << " thread(s)\n\n";

    auto seqStart = std::chrono::high_resolution_clock::now();
    long long seqResult = countPrimesSequential(N);
    auto seqEnd = std::chrono::high_resolution_clock::now();
    auto seqMs = std::chrono::duration_cast<std::chrono::milliseconds>(seqEnd - seqStart).count();
    std::cout << "Sequential result: " << seqResult << "  (" << seqMs << " ms)\n";

    auto parStart = std::chrono::high_resolution_clock::now();

    uint32_t sqrtN = (uint32_t)std::sqrt((double)N) + 1;
    std::vector<uint32_t> smallPrimes = smallPrimesUpTo(sqrtN); 

    std::vector<std::thread> threads;
    std::vector<long long> partialCounts(numThreads, 0);
    uint64_t chunkSize = (N - 1) / numThreads; 

    for (unsigned t = 0; t < numThreads; ++t) {
        uint64_t low = 2 + t * chunkSize;
        uint64_t high = (t == numThreads - 1) ? N : (low + chunkSize - 1);
        threads.emplace_back(sieveSegment, low, high, std::cref(smallPrimes),
                              std::ref(partialCounts[t]));
    }

    for (auto& th : threads) th.join(); 

    long long parResult = 0;
    for (long long c : partialCounts) parResult += c; 

    auto parEnd = std::chrono::high_resolution_clock::now();
    auto parMs = std::chrono::duration_cast<std::chrono::milliseconds>(parEnd - parStart).count();
    std::cout << "Parallel result:   " << parResult << "  (" << parMs << " ms)\n\n";

    std::cout << "Match: " << (seqResult == parResult ? "YES" : "NO (bug!)") << "\n";
    if (parMs > 0)
        std::cout << "Speedup: " << std::fixed << std::setprecision(2)
                   << (double)seqMs / (double)parMs << "x\n";

    return 0;
}
