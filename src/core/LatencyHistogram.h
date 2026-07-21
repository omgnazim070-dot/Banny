#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

class LatencyHistogram
{
public:

    static constexpr std::uint64_t
        bucketWidthUs = 10;

    static constexpr std::size_t
        regularBucketCount = 2000;

    static constexpr std::size_t
        bucketCount = regularBucketCount + 1;

    void Record(
        std::uint64_t valueUs)
    {
        std::size_t bucket =
            static_cast<std::size_t>(
                valueUs / bucketWidthUs);

        if (bucket >= regularBucketCount)
        {
            bucket = regularBucketCount;
        }

        buckets[bucket]++;
        samples++;
        totalUs += valueUs;
        maximumUs = (std::max)(
            maximumUs,
            valueUs);
    }

    void Merge(
        const LatencyHistogram& other)
    {
        for (std::size_t bucket = 0;
            bucket < bucketCount;
            ++bucket)
        {
            buckets[bucket] +=
                other.buckets[bucket];
        }

        samples += other.samples;
        totalUs += other.totalUs;
        maximumUs = (std::max)(
            maximumUs,
            other.maximumUs);
    }

    void Clear()
    {
        buckets.fill(0);
        samples = 0;
        totalUs = 0;
        maximumUs = 0;
    }

    std::uint64_t Count() const
    {
        return samples;
    }

    double AverageUs() const
    {
        return samples > 0
            ? static_cast<double>(totalUs) /
            static_cast<double>(samples)
            : 0.0;
    }

    std::uint64_t PercentileUs(
        double percentile) const
    {
        if (samples == 0)
        {
            return 0;
        }

        const std::uint64_t target =
            static_cast<std::uint64_t>(
                std::ceil(
                    percentile *
                    static_cast<double>(samples)));

        std::uint64_t accumulated = 0;

        for (std::size_t bucket = 0;
            bucket < bucketCount;
            ++bucket)
        {
            accumulated += buckets[bucket];

            if (accumulated < target)
            {
                continue;
            }

            if (bucket == regularBucketCount)
            {
                return maximumUs;
            }

            const std::uint64_t bucketUpperUs =
                (static_cast<std::uint64_t>(bucket) + 1) *
                bucketWidthUs - 1;

            return (std::min)(
                bucketUpperUs,
                maximumUs);
        }

        return maximumUs;
    }

    std::uint64_t MaximumUs() const
    {
        return maximumUs;
    }

private:

    std::array<std::uint64_t, bucketCount>
        buckets{};

    std::uint64_t samples = 0;

    std::uint64_t totalUs = 0;

    std::uint64_t maximumUs = 0;
};
