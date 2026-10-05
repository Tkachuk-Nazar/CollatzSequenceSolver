#include "CollatzSolver.h"
#include <thread>
#include <chrono>
#include <limits>

void CollatzSolver::requestStop()
{
    globalStopRequested.store(true);
}

void CollatzSolver::worker(uint64_t start, uint64_t end, ThreadResult& res)
{
    uint64_t localMaxLen = 0;
    uint64_t localMaxNum = 0;

    constexpr uint64_t maxUint64 = std::numeric_limits<uint64_t>::max();
    constexpr uint64_t overflowLimit = (maxUint64 - 1) / 3;
    for (uint64_t i = start; i <= end; ++i)
    {
        if (globalOverflowOccured.load() || globalStopRequested.load())
            return;

        uint64_t n = i;
        uint32_t currentLen = 1;

        while (n > 1)
        {
            if (n % 2 == 0)
            {
                n /= 2;
            }
            else
            {
                if (n > overflowLimit)
                {
                    res.overflowOccured = true;
                    globalOverflowOccured.store(true);
                    return;
                }
                n = 3 * n + 1;
            }
            currentLen++;
        }
        if (currentLen > localMaxLen)
        {
            localMaxLen = currentLen;
            localMaxNum = i;
        }
    }
    res.maxNumber = localMaxNum;
    res.maxLength = localMaxLen;
}

FinalResult CollatzSolver::solve(uint64_t maxLimit, unsigned int numThreads)
{
    globalOverflowOccured.store(false);
    globalStopRequested.store(false);

    FinalResult finalRes;

    auto startTime = std::chrono::high_resolution_clock::now();

    uint64_t chunkSize = maxLimit / numThreads;
    std::vector<std::thread> threads;

    std::vector<ThreadResult> results(numThreads);

    for (unsigned int i = 0; i < numThreads; ++i)
    {
        uint64_t start = i * chunkSize + 1;
        uint64_t end = (i == numThreads - 1) ? maxLimit : (start + chunkSize - 1);

        threads.push_back(std::thread(&CollatzSolver::worker, this, start, end, std::ref(results[i])));
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    finalRes.durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    if (globalStopRequested.load())
    {
        finalRes.stoppedByUser = true;
        return finalRes;
    }
    if (globalOverflowOccured.load())
    {
        finalRes.overflowOccured = true;
        return finalRes;
    }

    for (const auto& res : results)
    {
        if (finalRes.maxLength < res.maxLength)
        {
            finalRes.maxNumber= res.maxNumber;
            finalRes.maxLength = res.maxLength;
        }
    }

    return finalRes;
}