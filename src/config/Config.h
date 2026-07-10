#pragma once

#include <string>

class Config
{
public:
    bool Load();

    std::string BotName;
    std::string TradingMode;

    int ScanIntervalMs = 0;

    double MinProfitPercent = 0.0;
};