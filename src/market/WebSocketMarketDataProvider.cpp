#include "WebSocketMarketDataProvider.h"

#include "../../third_party/json/json.hpp"

#include <iostream>
#include <vector>
#include <chrono>

using json = nlohmann::json;

namespace
{
    void WorkerLoop(
        int clientId,
        BinanceWebSocketClient* client,
        MarketDataCache* cache,
        RealtimeIndexer* realtimeIndexer,
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
                static int parseErrors = 0;

                if (parseErrors < 10)
                {
                    std::cout
                        << "JSON PARSE ERROR:"
                        << std::endl;

                    std::cout
                        << message
                        << std::endl
                        << std::endl;
                }

                ++parseErrors;

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
                static int invalidPayload = 0;

                if (invalidPayload < 10)
                {
                    std::cout
                        << "INVALID PAYLOAD:"
                        << std::endl;

                    std::cout
                        << payload.dump(2)
                        << std::endl
                        << std::endl;
                }

                ++invalidPayload;

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

            if (realtimeIndexer != nullptr)
            {
                realtimeIndexer->OnTicker(
                    ticker.symbol,
                    ticker.bidPrice,
                    ticker.askPrice);
            }

            static std::atomic<int> updates[8];

            int count = ++updates[clientId - 1];

            if (count % 5000 == 0)
            {
                std::cout
                    << "Client "
                    << clientId
                    << " received "
                    << count
                    << " ticker updates"
                    << std::endl;
            }
        }
    }
}

bool WebSocketMarketDataProvider::Start(
    const std::vector<std::string>& symbols)
{
    std::vector<std::string> symbols1;
    std::vector<std::string> symbols2;
    std::vector<std::string> symbols3;
    std::vector<std::string> symbols4;
    std::vector<std::string> symbols5;
    std::vector<std::string> symbols6;
    std::vector<std::string> symbols7;
    std::vector<std::string> symbols8;

    size_t part =
        symbols.size() / 8;

    for (size_t i = 0; i < symbols.size(); ++i)
    {
        if (i < part)
        {
            symbols1.push_back(symbols[i]);
        }
        else if (i < part * 2)
        {
            symbols2.push_back(symbols[i]);
        }
        else if (i < part * 3)
        {
            symbols3.push_back(symbols[i]);
        }
        else if (i < part * 4)
        {
            symbols4.push_back(symbols[i]);
        }
        else if (i < part * 5)
        {
            symbols5.push_back(symbols[i]);
        }
        else if (i < part * 6)
        {
            symbols6.push_back(symbols[i]);
        }
        else if (i < part * 7)
        {
            symbols7.push_back(symbols[i]);
        }
        else
        {
            symbols8.push_back(symbols[i]);
        }
    }

    std::cout << "Client1 symbols: " << symbols1.size() << std::endl;
    std::cout << "Client2 symbols: " << symbols2.size() << std::endl;
    std::cout << "Client3 symbols: " << symbols3.size() << std::endl;
    std::cout << "Client4 symbols: " << symbols4.size() << std::endl;
    std::cout << "Client5 symbols: " << symbols5.size() << std::endl;
    std::cout << "Client6 symbols: " << symbols6.size() << std::endl;
    std::cout << "Client7 symbols: " << symbols7.size() << std::endl;
    std::cout << "Client8 symbols: " << symbols8.size() << std::endl;

    std::cout << "\nClient8 first: "
        << symbols8.front()
        << std::endl;

    std::cout << "Client8 last: "
        << symbols8.back()
        << std::endl;

    std::cout << "\n===== CLIENT RANGES =====\n";

    auto printRange = [](const std::string& name,
        const std::vector<std::string>& list)
        {
            if (list.empty())
                return;

            std::cout
                << name
                << " | first = "
                << list.front()
                << " | last = "
                << list.back()
                << std::endl;
        };

    printRange("Client1", symbols1);
    printRange("Client2", symbols2);
    printRange("Client3", symbols3);
    printRange("Client4", symbols4);
    printRange("Client5", symbols5);
    printRange("Client6", symbols6);
    printRange("Client7", symbols7);
    printRange("Client8", symbols8);

    std::cout << "\n=== Connecting OBJECT client1 ===\n";
    if (!client1.Connect(symbols1)) return false;

    std::cout << "\n=== Connecting OBJECT client2 ===\n";
    if (!client2.Connect(symbols2)) return false;

    std::cout << "\n=== Connecting OBJECT client3 ===\n";
    if (!client3.Connect(symbols3)) return false;

    std::cout << "\n=== Connecting OBJECT client4 ===\n";
    if (!client4.Connect(symbols4)) return false;

    std::cout << "\n=== Connecting OBJECT client5 ===\n";
    if (!client5.Connect(symbols5)) return false;

    std::cout << "\n=== Connecting OBJECT client7 (using symbols6) ===\n";
    if (!client7.Connect(symbols6)) return false;

    std::cout << "\n=== Connecting OBJECT client6 (using symbols7) ===\n";
    if (!client6.Connect(symbols7))
    {
        return false;
    }

    if (!client8.Connect(symbols8))
    {
        std::cout
            << "Client8 failed"
            << std::endl;
    }

    running = true;

    worker1 = std::thread(WorkerLoop, 1, &client1, &cache, realtimeIndexer, &running);
    worker2 = std::thread(WorkerLoop, 2, &client2, &cache, realtimeIndexer, &running);
    worker3 = std::thread(WorkerLoop, 3, &client3, &cache, realtimeIndexer, &running);
    worker4 = std::thread(WorkerLoop, 4, &client4, &cache, realtimeIndexer, &running);
    worker5 = std::thread(WorkerLoop, 5, &client5, &cache, realtimeIndexer, &running);
    worker6 = std::thread(WorkerLoop, 6, &client6, &cache, realtimeIndexer, &running);
    worker7 = std::thread(WorkerLoop, 7, &client7, &cache, realtimeIndexer, &running);
    worker8 = std::thread(WorkerLoop, 8, &client8, &cache, realtimeIndexer, &running);

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

    processClient(
        client3);

    processClient(
        client4);

    processClient(
        client5);

    processClient(
        client6);

    processClient(
        client7);

    processClient(
        client8);
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

    if (worker3.joinable())
    {
        worker3.join();
    }

    if (worker4.joinable())
    {
        worker4.join();
    }

    if (worker5.joinable())
    {
        worker5.join();
    }

    if (worker6.joinable())
    {
        worker6.join();
    }

    if (worker7.joinable())
    {
        worker7.join();
    }

    if (worker8.joinable())
    {
        worker8.join();
    }

    client1.Disconnect();
    client2.Disconnect();
    client3.Disconnect();
    client4.Disconnect();
    client5.Disconnect();
    client6.Disconnect();
    client7.Disconnect();
    client8.Disconnect();
}

void WebSocketMarketDataProvider::SetRealtimeIndexer(
    RealtimeIndexer* indexer)
{
    realtimeIndexer = indexer;
}