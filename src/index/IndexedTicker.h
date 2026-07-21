#pragma once

#include <chrono>
#include <cstdint>

struct IndexedTicker
{
    double bidPrice = 0.0;
    double askPrice = 0.0;

    std::chrono::steady_clock::time_point updatedAt{};

    std::uint64_t version = 0;

    bool initialized = false;
    bool valid = false;
};