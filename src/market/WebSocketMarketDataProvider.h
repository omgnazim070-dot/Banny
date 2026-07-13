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

    BinanceWebSocketClient client3;

    BinanceWebSocketClient client4;

    BinanceWebSocketClient client5;

    BinanceWebSocketClient client6;

    BinanceWebSocketClient client7;

    BinanceWebSocketClient client8;

    MarketDataCache cache;

    std::thread worker1;

    std::thread worker2;

    std::thread worker3;

    std::thread worker4;

    std::thread worker5;

    std::thread worker6;

    std::thread worker7;

    std::thread worker8;

    std::atomic<bool> running = false;
};