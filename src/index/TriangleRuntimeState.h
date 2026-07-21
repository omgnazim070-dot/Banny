#pragma once

#include <cstdint>

struct TriangleRuntimeState
{
    std::uint64_t version1 = 0;
    std::uint64_t version2 = 0;
    std::uint64_t version3 = 0;

    double lastProfitPercent = 0.0;

    std::int64_t lastCalculationNs = 0;

    bool hasVersions = false;
};
