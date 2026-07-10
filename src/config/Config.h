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

    double CommissionPercent = 0.0;
    double SlippagePercent = 0.0;

    double StartBalance = 1000.0;
};