#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

#include "SymbolId.h"

struct DirtyTask
{
    TriangleId triangle = 0;

    std::int64_t eventTimeNs = 0;

    std::int64_t queuedAtNs = 0;
};

class DirtyQueue
{
public:

    explicit DirtyQueue(
        std::size_t triangleCount = 0);

    void Mark(
        TriangleId triangle,
        std::int64_t eventTimeNs);

    void MarkBatch(
        const std::vector<TriangleId>& triangles,
        std::int64_t eventTimeNs);

    bool WaitAndDrain(
        std::vector<DirtyTask>& tasks,
        std::size_t maxCount,
        std::chrono::steady_clock::time_point deadline,
        bool spinBeforeWait,
        std::uint64_t& spinWaitUs);

    bool FinishProcessing(
        TriangleId triangle,
        bool allowImmediateRecheck,
        DirtyTask& recheckTask);

    void Clear();

    std::uint64_t TakeAcceptedMarks();

    std::uint64_t TakeCoalescedMarks();

    std::uint64_t TakeImmediateRechecks();

    std::uint64_t TakeDeferredRechecks();

    std::size_t Size() const;

    std::size_t TakePeakDepth();

    std::uint64_t OldestQueuedAgeUs(
        std::int64_t nowNs) const;

private:

    enum class TriangleState : std::uint8_t
    {
        Idle,
        Queued,
        Processing,
        ProcessingNeedsRecheck
    };

    bool PushLocked(
        TriangleId triangle);

    TriangleId PopLocked();

    void UpdatePeakDepth(
        std::size_t currentDepth);

    std::vector<TriangleId> ring;

    std::size_t head = 0;

    std::size_t tail = 0;

    std::size_t queuedCount = 0;

    std::vector<TriangleState> states;

    std::vector<std::int64_t>
        eventTimesNs;

    std::vector<std::int64_t>
        queuedAtTimesNs;

    mutable std::mutex mutex;

    std::condition_variable condition;

    std::atomic<std::size_t>
        currentDepth{ 0 };

    std::atomic<std::uint64_t>
        acceptedMarks{ 0 };

    std::atomic<std::uint64_t>
        coalescedMarks{ 0 };

    std::atomic<std::uint64_t>
        immediateRechecks{ 0 };

    std::atomic<std::uint64_t>
        deferredRechecks{ 0 };

    std::atomic<std::size_t>
        peakDepth{ 0 };
};
