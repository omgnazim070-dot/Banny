#pragma once

#include "../binance/BinanceWebSocketClient.h"
#include "MarketDataCache.h"

#include <vector>
#include <string>
#include <thread>
#include <atomic>

class WebSocketMarketDataProvider
{
public:

    bool Start(
        const std::vector<std::string>& symbols);

    void ProcessNextMessage();

    MarketData GetSnapshot();

    void Stop();

private:

    BinanceWebSocketClient client1;

    BinanceWebSocketClient client2;

    MarketDataCache cache;

    std::thread worker1;

    std::thread worker2;

    std::atomic<bool> running = false;
};