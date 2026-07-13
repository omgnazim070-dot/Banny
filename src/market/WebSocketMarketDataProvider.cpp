#include "WebSocketMarketDataProvider.h"

#include "../../third_party/json/json.hpp"

#include <iostream>
#include <vector>
#include <chrono>

using json = nlohmann::json;

namespace
{
    void WorkerLoop(
        BinanceWebSocketClient* client,
        MarketDataCache* cache,
        std::atomic<bool>* running)
    {
        while (running->load())
        {
            std::string message;

            if (!client->Receive(message))
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(100));

                continue;
            }

            json data;

            try
            {
                data =
                    json::parse(message);
            }
            catch (...)
            {
                continue;
            }

            if (!data.contains("data"))
            {
                continue;
            }

            auto payload =
                data["data"];

            if (!payload.contains("s") ||
                !payload.contains("b") ||
                !payload.contains("a"))
            {
                continue;
            }

            Ticker ticker;

            ticker.symbol =
                payload["s"].get<std::string>();

            ticker.bidPrice =
                std::stod(
                    payload["b"].get<std::string>());

            ticker.askPrice =
                std::stod(
                    payload["a"].get<std::string>());

            cache->Update(ticker);
        }
    }
}

bool WebSocketMarketDataProvider::Start(
    const std::vector<std::string>& symbols)
{
    std::vector<std::string> symbols1;

    std::vector<std::string> symbols2;

    for (size_t i = 0; i < symbols.size(); ++i)
    {
        if (i < symbols.size() / 2)
        {
            symbols1.push_back(
                symbols[i]);
        }
        else
        {
            symbols2.push_back(
                symbols[i]);
        }
    }

    std::cout
        << "Client1 symbols: "
        << symbols1.size()
        << std::endl;

    std::cout
        << "Client2 symbols: "
        << symbols2.size()
        << std::endl;

    if (!client1.Connect(symbols1))
    {
        return false;
    }

    if (!client2.Connect(symbols2))
    {
        return false;
    }

    running = true;

    worker1 =
        std::thread(
            WorkerLoop,
            &client1,
            &cache,
            &running);

    worker2 =
        std::thread(
            WorkerLoop,
            &client2,
            &cache,
            &running);

    return true;
}

void WebSocketMarketDataProvider::ProcessNextMessage()
{
    auto processClient =
        [this](BinanceWebSocketClient& client)
        {
            std::string message;

            if (!client.Receive(message))
            {
                return;
            }

            json data;

            try
            {
                data =
                    json::parse(message);
            }
            catch (...)
            {
                return;
            }

            if (!data.contains("data"))
            {
                return;
            }

            auto payload =
                data["data"];

            if (!payload.contains("s") ||
                !payload.contains("b") ||
                !payload.contains("a"))
            {
                return;
            }

            Ticker ticker;

            ticker.symbol =
                payload["s"].get<std::string>();

            ticker.bidPrice =
                std::stod(
                    payload["b"].get<std::string>());

            ticker.askPrice =
                std::stod(
                    payload["a"].get<std::string>());

            cache.Update(
                ticker);

        };

    processClient(
        client1);

    processClient(
        client2);
}

MarketData WebSocketMarketDataProvider::GetSnapshot()
{
    return cache.GetSnapshot();
}

void WebSocketMarketDataProvider::Stop()
{
    running = false;

    if (worker1.joinable())
    {
        worker1.join();
    }

    if (worker2.joinable())
    {
        worker2.join();
    }

    client1.Disconnect();
    client2.Disconnect();
}