#include "Config.h"

#include "../../third_party/json/json.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool Config::Load()
{
    std::ifstream file("C:/Projects/Banny/config/config.json");

    std::cout
        << "CONFIG PATH: C:/Projects/Banny/config/config.json"
        << std::endl;

    if (!file.is_open())
    {
        std::cout
            << "Failed to open config.json"
            << std::endl;

        return false;
    }

    json config;

    try
    {
        file >> config;
    }
    catch (const std::exception& ex)
    {
        std::cout
            << "JSON ERROR: "
            << ex.what()
            << std::endl;

        return false;
    }

    std::cout
        << "Loaded JSON:"
        << std::endl;

    std::cout
        << config.dump(4)
        << std::endl;

    BotName = config.value("bot_name", "Banny");

    TradingMode = config.value(
        "trading_mode",
        "paper"
    );

    ScanIntervalMs = config.value(
        "scan_interval_ms",
        30
    );

    MinProfitPercent = config.value(
        "min_profit_percent",
        0.20
    );

    std::cout
        << "Loading config.json"
        << std::endl;

    return true;
}