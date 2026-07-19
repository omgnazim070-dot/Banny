#pragma once

#include <string>
#include <chrono>

struct Ticker
{
    std::string symbol;

    double bidPrice = 0.0;
    double askPrice = 0.0;

    std::chrono::steady_clock::time_point lastUpdate;
};