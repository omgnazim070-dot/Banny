#include "DirtyQueue.h"

#include <algorithm>
#include <thread>

#if defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__)
#include <immintrin.h>
#endif

namespace
{
    std::int64_t GetNowNs()
    {
        return std::chrono::duration_cast<
            std::chrono::nanoseconds>(
                std::chrono::steady_clock::now()
                .time_since_epoch())
            .count();
    }

    void CpuRelax()
    {
#if defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__)
        _mm_pause();
#else
        std::this_thread::yield();
#endif
    }
}

DirtyQueue::DirtyQueue(
    std::size_t triangleCount)
    :
    ring(triangleCount),
    states(
        triangleCount,
        TriangleState::Idle),
    eventTimesNs(
        triangleCount,
        0),
    queuedAtTimesNs(
        triangleCount,
        0)
{
}

bool DirtyQueue::PushLocked(
    TriangleId triangle)
{
    if (ring.empty() ||
        queuedCount >= ring.size())
    {
        return false;
    }

    ring[tail] = triangle;

    tail++;

    if (tail == ring.size())
    {
        tail = 0;
    }

    queuedCount++;

    currentDepth.store(
        queuedCount,
        std::memory_order_release);

    return true;
}

TriangleId DirtyQueue::PopLocked()
{
    const TriangleId triangle =
        ring[head];

    head++;

    if (head == ring.size())
    {
        head = 0;
    }

    queuedCount--;

    currentDepth.store(
        queuedCount,
        std::memory_order_release);

    return triangle;
}

void DirtyQueue::UpdatePeakDepth(
    std::size_t depth)
{
    std::size_t observedPeak =
        peakDepth.load(
            std::memory_order_relaxed);

    while (depth > observedPeak &&
        !peakDepth.compare_exchange_weak(
            observedPeak,
            depth,
            std::memory_order_relaxed))
    {
    }
}

void DirtyQueue::Mark(
    TriangleId triangle,
    std::int64_t eventTimeNs)
{
    bool accepted = false;
    bool coalesced = false;
    std::size_t depth = 0;

    const std::int64_t queuedAtNs =
        GetNowNs();

    {
        std::lock_guard<std::mutex> lock(
            mutex);

        if (triangle >= states.size())
        {
            return;
        }

        TriangleState& state =
            states[triangle];

        if (state != TriangleState::Idle)
        {
            if (eventTimeNs >
                eventTimesNs[triangle])
            {
                eventTimesNs[triangle] =
                    eventTimeNs;
            }

            if (state == TriangleState::Processing)
            {
                state =
                    TriangleState::ProcessingNeedsRecheck;
            }

            coalesced = true;
        }
        else
        {
            state = TriangleState::Queued;

            eventTimesNs[triangle] =
                eventTimeNs;

            queuedAtTimesNs[triangle] =
                queuedAtNs;

            if (PushLocked(triangle))
            {
                accepted = true;
                depth = queuedCount;
            }
            else
            {
                state = TriangleState::Idle;
                eventTimesNs[triangle] = 0;
                queuedAtTimesNs[triangle] = 0;
            }
        }
    }

    if (coalesced)
    {
        coalescedMarks.fetch_add(
            1,
            std::memory_order_relaxed);
    }

    if (accepted)
    {
        acceptedMarks.fetch_add(
            1,
            std::memory_order_relaxed);

        UpdatePeakDepth(
            depth);

        condition.notify_one();
    }
}

void DirtyQueue::MarkBatch(
    const std::vector<TriangleId>& triangles,
    std::int64_t eventTimeNs)
{
    if (triangles.empty())
    {
        return;
    }

    std::uint64_t accepted = 0;
    std::uint64_t coalesced = 0;
    std::size_t depth = 0;

    const std::int64_t queuedAtNs =
        GetNowNs();

    {
        std::lock_guard<std::mutex> lock(
            mutex);

        for (const TriangleId triangle :
            triangles)
        {
            if (triangle >= states.size())
            {
                continue;
            }

            TriangleState& state =
                states[triangle];

            if (state != TriangleState::Idle)
            {
                if (eventTimeNs >
                    eventTimesNs[triangle])
                {
                    eventTimesNs[triangle] =
                        eventTimeNs;
                }

                if (state == TriangleState::Processing)
                {
                    state =
                        TriangleState::ProcessingNeedsRecheck;
                }

                coalesced++;
                continue;
            }

            state = TriangleState::Queued;

            eventTimesNs[triangle] =
                eventTimeNs;

            queuedAtTimesNs[triangle] =
                queuedAtNs;

            if (PushLocked(triangle))
            {
                accepted++;
            }
            else
            {
                state = TriangleState::Idle;
                eventTimesNs[triangle] = 0;
                queuedAtTimesNs[triangle] = 0;
            }
        }

        depth = queuedCount;
    }

    if (coalesced > 0)
    {
        coalescedMarks.fetch_add(
            coalesced,
            std::memory_order_relaxed);
    }

    if (accepted > 0)
    {
        acceptedMarks.fetch_add(
            accepted,
            std::memory_order_relaxed);

        UpdatePeakDepth(
            depth);

        condition.notify_one();
    }
}

bool DirtyQueue::WaitAndDrain(
    std::vector<DirtyTask>& tasks,
    std::size_t maxCount,
    std::chrono::steady_clock::time_point deadline,
    bool spinBeforeWait,
    std::uint64_t& spinWaitUs)
{
    tasks.clear();
    spinWaitUs = 0;

    if (spinBeforeWait &&
        currentDepth.load(
            std::memory_order_acquire) == 0)
    {
        const auto spinStart =
            std::chrono::steady_clock::now();

        const auto spinDeadline =
            spinStart +
            std::chrono::microseconds(20);

        while (currentDepth.load(
            std::memory_order_acquire) == 0 &&
            std::chrono::steady_clock::now() <
            spinDeadline)
        {
            CpuRelax();
        }

        spinWaitUs =
            static_cast<std::uint64_t>(
                std::chrono::duration_cast<
                    std::chrono::microseconds>(
                        std::chrono::steady_clock::now() -
                        spinStart)
                    .count());
    }

    std::unique_lock<std::mutex> lock(
        mutex);

    const bool hasData =
        condition.wait_until(
            lock,
            deadline,
            [this]()
            {
                return queuedCount > 0;
            });

    if (!hasData)
    {
        return false;
    }

    while (queuedCount > 0 &&
        tasks.size() < maxCount)
    {
        const TriangleId triangle =
            PopLocked();

        if (triangle >= states.size())
        {
            continue;
        }

        DirtyTask task;

        task.triangle =
            triangle;

        task.eventTimeNs =
            eventTimesNs[triangle];

        task.queuedAtNs =
            queuedAtTimesNs[triangle];

        states[triangle] =
            TriangleState::Processing;

        tasks.push_back(
            task);
    }

    return !tasks.empty();
}

bool DirtyQueue::FinishProcessing(
    TriangleId triangle,
    bool allowImmediateRecheck,
    DirtyTask& recheckTask)
{
    bool notify = false;
    bool immediate = false;
    std::size_t depth = 0;

    const std::int64_t nowNs =
        GetNowNs();

    {
        std::lock_guard<std::mutex> lock(
            mutex);

        if (triangle >= states.size())
        {
            return false;
        }

        TriangleState& state =
            states[triangle];

        if (state ==
            TriangleState::ProcessingNeedsRecheck)
        {
            if (allowImmediateRecheck)
            {
                state = TriangleState::Processing;

                recheckTask.triangle =
                    triangle;

                recheckTask.eventTimeNs =
                    eventTimesNs[triangle];

                recheckTask.queuedAtNs =
                    nowNs;

                queuedAtTimesNs[triangle] =
                    nowNs;

                immediate = true;
            }
            else
            {
                state = TriangleState::Queued;

                queuedAtTimesNs[triangle] =
                    nowNs;

                if (PushLocked(triangle))
                {
                    depth = queuedCount;
                    notify = true;
                }
                else
                {
                    state = TriangleState::Idle;
                    eventTimesNs[triangle] = 0;
                    queuedAtTimesNs[triangle] = 0;
                }
            }
        }
        else
        {
            state = TriangleState::Idle;
            eventTimesNs[triangle] = 0;
            queuedAtTimesNs[triangle] = 0;
        }
    }

    if (immediate)
    {
        immediateRechecks.fetch_add(
            1,
            std::memory_order_relaxed);

        return true;
    }

    if (notify)
    {
        deferredRechecks.fetch_add(
            1,
            std::memory_order_relaxed);

        UpdatePeakDepth(
            depth);

        condition.notify_one();
    }

    return false;
}

void DirtyQueue::Clear()
{
    std::lock_guard<std::mutex> lock(
        mutex);

    head = 0;
    tail = 0;
    queuedCount = 0;

    std::fill(
        states.begin(),
        states.end(),
        TriangleState::Idle);

    std::fill(
        eventTimesNs.begin(),
        eventTimesNs.end(),
        0);

    std::fill(
        queuedAtTimesNs.begin(),
        queuedAtTimesNs.end(),
        0);

    currentDepth.store(
        0,
        std::memory_order_relaxed);

    acceptedMarks.store(
        0,
        std::memory_order_relaxed);

    coalescedMarks.store(
        0,
        std::memory_order_relaxed);

    immediateRechecks.store(
        0,
        std::memory_order_relaxed);

    deferredRechecks.store(
        0,
        std::memory_order_relaxed);

    peakDepth.store(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
DirtyQueue::TakeAcceptedMarks()
{
    return acceptedMarks.exchange(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
DirtyQueue::TakeCoalescedMarks()
{
    return coalescedMarks.exchange(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
DirtyQueue::TakeImmediateRechecks()
{
    return immediateRechecks.exchange(
        0,
        std::memory_order_relaxed);
}

std::uint64_t
DirtyQueue::TakeDeferredRechecks()
{
    return deferredRechecks.exchange(
        0,
        std::memory_order_relaxed);
}

std::size_t DirtyQueue::Size() const
{
    return currentDepth.load(
        std::memory_order_acquire);
}

std::size_t DirtyQueue::TakePeakDepth()
{
    const std::size_t depth =
        Size();

    const std::size_t recordedPeak =
        peakDepth.exchange(
            depth,
            std::memory_order_relaxed);

    return recordedPeak > depth
        ? recordedPeak
        : depth;
}

std::uint64_t DirtyQueue::OldestQueuedAgeUs(
    std::int64_t nowNs) const
{
    std::lock_guard<std::mutex> lock(
        mutex);

    if (queuedCount == 0 ||
        ring.empty())
    {
        return 0;
    }

    const TriangleId triangle =
        ring[head];

    if (triangle >=
        queuedAtTimesNs.size())
    {
        return 0;
    }

    const std::int64_t queuedAtNs =
        queuedAtTimesNs[triangle];

    if (queuedAtNs <= 0 ||
        nowNs <= queuedAtNs)
    {
        return 0;
    }

    return static_cast<std::uint64_t>(
        (nowNs - queuedAtNs) / 1000);
}
