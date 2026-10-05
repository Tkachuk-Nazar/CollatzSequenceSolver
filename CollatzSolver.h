#ifndef _COLLATZ_SOLVER_H_
#define _COLLATZ_SOLVER_H_

#include <vector>
#include <cstdint>
#include <atomic>

struct ThreadResult
{
    uint64_t maxNumber = 0;
    uint32_t maxLength = 0;
    bool overflowOccured = false;
};

struct FinalResult
{
    uint64_t maxNumber = 0;
    uint32_t maxLength = 0;
    uint64_t durationMs = 0;
    bool overflowOccured = false;
    bool stoppedByUser = false;
};

class CollatzSolver
{
public:
    CollatzSolver() = default;
    ~CollatzSolver() = default;

    FinalResult solve(uint64_t maxLimit, unsigned int numThreads);

    void requestStop();
private:
    void worker(uint64_t start, uint64_t end, ThreadResult& res);

    std::atomic<bool> globalOverflowOccured{ false };
    std::atomic<bool> globalStopRequested{ false };
};

#endif // _COLLATZ_SOLVER_H_