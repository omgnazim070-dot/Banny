#include "WebSocketMarketDataProvider.h"
#include "BinanceMarketDataProvider.h"

#include <charconv>
#include <chrono>
#include <iostream>
#include <string_view>
#include <system_error>
#include <vector>

namespace
{
    std::int64_t GetNowNs()
    {
        return std::chrono::duration_cast<
            std::chrono::nanoseconds>(
                std::chrono::steady_clock::now()
                .time_since_epoch())
            .count();
    }

    std::uint64_t ClampDurationUs(
        std::int64_t durationNs)
    {
        if (durationNs <= 0)
        {
            return 0;
        }

        return static_cast<std::uint64_t>(
            durationNs / 1000);
    }

    bool FindQuotedField(
        std::string_view json,
        std::string_view fieldToken,
        std::string_view& value)
    {
        const std::size_t fieldPosition =
            json.find(
                fieldToken);

        if (fieldPosition ==
            std::string_view::npos)
        {
            return false;
        }

        std::size_t position =
            json.find(
                ':',
                fieldPosition +
                fieldToken.size());

        if (position ==
            std::string_view::npos)
        {
            return false;
        }

        position++;

        while (position < json.size() &&
            (json[position] == ' ' ||
                json[position] == '\t' ||
                json[position] == '\r' ||
                json[position] == '\n'))
        {
            position++;
        }

        if (position >= json.size() ||
            json[position] != '"')
        {
            return false;
        }

        const std::size_t valueStart =
            position + 1;

        const std::size_t valueEnd =
            json.find(
                '"',
                valueStart);

        if (valueEnd ==
            std::string_view::npos)
        {
            return false;
        }

        value = json.substr(
            valueStart,
            valueEnd - valueStart);

        return true;
    }

    bool ParseDouble(
        std::string_view text,
        double& value)
    {
        const char* begin =
            text.data();

        const char* end =
            text.data() + text.size();

        const auto result =
            std::from_chars(
                begin,
                end,
                value,
                std::chars_format::general);

        return result.ec == std::errc{} &&
            result.ptr == end;
    }

    bool ParseBookTicker(
        const std::string& message,
        std::string_view& symbol,
        double& bidPrice,
        double& askPrice)
    {
        const std::string_view json(
            message);

        std::string_view payload =
            json;

        const std::size_t dataPosition =
            json.find(
                "\"data\"");

        if (dataPosition !=
            std::string_view::npos)
        {
            const std::size_t objectStart =
                json.find(
                    '{',
                    dataPosition + 6);

            if (objectStart ==
                std::string_view::npos)
            {
                return false;
            }

            payload = json.substr(
                objectStart);
        }

        std::string_view bidText;
        std::string_view askText;

        if (!FindQuotedField(
                payload,
                "\"s\"",
                symbol) ||
            !FindQuotedField(
                payload,
                "\"b\"",
                bidText) ||
            !FindQuotedField(
                payload,
                "\"a\"",
                askText))
        {
            return false;
        }

        return ParseDouble(
                bidText,
                bidPrice) &&
            ParseDouble(
                askText,
                askPrice);
    }

    void PublishStats(
        WebSocketIntervalStats& localStats,
        WebSocketMarketDataProvider::WorkerStatsSlot* publishSlot)
    {
        if (publishSlot == nullptr ||
            localStats.receivedMessages == 0)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(
            publishSlot->mutex);

        publishSlot->pending.Merge(
            localStats);

        localStats.Clear();
    }

    void RestoreSymbolsFromRest(
        const std::vector<std::string>& symbols,
        MarketDataCache* cache,
        RealtimeIndexer* realtimeIndexer)
    {
        BinanceMarketDataProvider restProvider;

        const auto snapshot =
            restProvider.GetMarketData();

        std::size_t restored = 0;

        for (const auto& symbol :
            symbols)
        {
            const auto tickerIt =
                snapshot.tickers.find(
                    symbol);

            if (tickerIt ==
                snapshot.tickers.end())
            {
                continue;
            }

            const auto& ticker =
                tickerIt->second;

            if (realtimeIndexer != nullptr)
            {
                realtimeIndexer->OnTicker(
                    ticker.symbol,
                    ticker.bidPrice,
                    ticker.askPrice);
            }
            else if (cache != nullptr)
            {
                cache->Update(
                    ticker);
            }

            restored++;
        }

        std::cout
            << "REST restored symbols: "
            << restored
            << " / "
            << symbols.size()
            << std::endl;
    }

    void WorkerLoop(
        int clientId,
        BinanceWebSocketClient* client,
        const std::vector<std::string> symbols,
        MarketDataCache* cache,
        RealtimeIndexer* realtimeIndexer,
        WebSocketMarketDataProvider::WorkerStatsSlot* publishSlot,
        std::atomic<bool>* running)
    {
        WebSocketIntervalStats localStats;

        auto nextPublishTime =
            std::chrono::steady_clock::now() +
            std::chrono::milliseconds(100);

        while (running->load(
            std::memory_order_acquire))
        {
            std::string message;

            if (!client->Receive(message))
            {
                PublishStats(
                    localStats,
                    publishSlot);

                if (!running->load(
                    std::memory_order_acquire))
                {
                    break;
                }

                if (realtimeIndexer != nullptr)
                {
                    realtimeIndexer->OnDisconnected(
                        symbols);
                }

                client->Disconnect();

                while (running->load(
                    std::memory_order_acquire))
                {
                    std::cout
                        << "Client "
                        << clientId
                        << " reconnecting..."
                        << std::endl;

                    std::this_thread::sleep_for(
                        std::chrono::seconds(1));

                    if (!running->load(
                        std::memory_order_acquire))
                    {
                        break;
                    }

                    if (client->Connect(symbols))
                    {
                        RestoreSymbolsFromRest(
                            symbols,
                            cache,
                            realtimeIndexer);

                        std::cout
                            << "Client "
                            << clientId
                            << " reconnected"
                            << std::endl;

                        break;
                    }

                    client->Disconnect();
                }

                nextPublishTime =
                    std::chrono::steady_clock::now() +
                    std::chrono::milliseconds(100);

                continue;
            }

            const std::int64_t receiveTimeNs =
                GetNowNs();

            localStats.receivedMessages++;

            std::string_view symbol;
            double bidPrice = 0.0;
            double askPrice = 0.0;

            if (!ParseBookTicker(
                message,
                symbol,
                bidPrice,
                askPrice))
            {
                localStats.parseErrors++;
            }
            else
            {
                localStats.parsedMessages++;

                if (realtimeIndexer != nullptr)
                {
                    const TickerUpdateResult result =
                        realtimeIndexer->OnTicker(
                            symbol,
                            bidPrice,
                            askPrice,
                            receiveTimeNs);

                    if (result ==
                        TickerUpdateResult::Changed)
                    {
                        localStats.priceChanges++;
                    }
                    else if (result ==
                        TickerUpdateResult::Unchanged)
                    {
                        localStats.unchangedPricesSkipped++;
                    }
                    else
                    {
                        localStats.unknownSymbols++;
                    }
                }
                else if (cache != nullptr)
                {
                    Ticker ticker;

                    ticker.symbol =
                        std::string(symbol);

                    ticker.bidPrice =
                        bidPrice;

                    ticker.askPrice =
                        askPrice;

                    cache->Update(
                        ticker);
                }

                localStats.receiveToIndex.Record(
                    ClampDurationUs(
                        GetNowNs() -
                        receiveTimeNs));
            }

            const auto currentTime =
                std::chrono::steady_clock::now();

            if (currentTime >=
                nextPublishTime)
            {
                PublishStats(
                    localStats,
                    publishSlot);

                do
                {
                    nextPublishTime +=
                        std::chrono::milliseconds(100);
                }
                while (currentTime >=
                    nextPublishTime);
            }
        }

        PublishStats(
            localStats,
            publishSlot);
    }
}

WebSocketMarketDataProvider::WebSocketMarketDataProvider()
{
    for (auto& slot :
        workerStatsSlots)
    {
        slot =
            std::make_unique<WorkerStatsSlot>();
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

    const std::size_t part =
        symbols.size() / 8;

    for (std::size_t i = 0;
        i < symbols.size();
        ++i)
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

    if (!symbols8.empty())
    {
        std::cout << "\nClient8 first: "
            << symbols8.front()
            << std::endl;

        std::cout << "Client8 last: "
            << symbols8.back()
            << std::endl;
    }

    std::cout << "\n===== CLIENT RANGES =====\n";

    const auto printRange =
        [](const std::string& name,
            const std::vector<std::string>& list)
        {
            if (list.empty())
            {
                return;
            }

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

    RestoreSymbolsFromRest(
        symbols,
        &cache,
        realtimeIndexer);

    running.store(
        true,
        std::memory_order_release);

    worker1 = std::thread(
        WorkerLoop,
        1,
        &client1,
        symbols1,
        &cache,
        realtimeIndexer,
        workerStatsSlots[0].get(),
        &running);

    worker2 = std::thread(
        WorkerLoop,
        2,
        &client2,
        symbols2,
        &cache,
        realtimeIndexer,
        workerStatsSlots[1].get(),
        &running);

    worker3 = std::thread(
        WorkerLoop,
        3,
        &client3,
        symbols3,
        &cache,
        realtimeIndexer,
        workerStatsSlots[2].get(),
        &running);

    worker4 = std::thread(
        WorkerLoop,
        4,
        &client4,
        symbols4,
        &cache,
        realtimeIndexer,
        workerStatsSlots[3].get(),
        &running);

    worker5 = std::thread(
        WorkerLoop,
        5,
        &client5,
        symbols5,
        &cache,
        realtimeIndexer,
        workerStatsSlots[4].get(),
        &running);

    worker6 = std::thread(
        WorkerLoop,
        6,
        &client6,
        symbols7,
        &cache,
        realtimeIndexer,
        workerStatsSlots[5].get(),
        &running);

    worker7 = std::thread(
        WorkerLoop,
        7,
        &client7,
        symbols6,
        &cache,
        realtimeIndexer,
        workerStatsSlots[6].get(),
        &running);

    worker8 = std::thread(
        WorkerLoop,
        8,
        &client8,
        symbols8,
        &cache,
        realtimeIndexer,
        workerStatsSlots[7].get(),
        &running);

    return true;
}

void WebSocketMarketDataProvider::ProcessNextMessage()
{
    const auto processClient =
        [this](BinanceWebSocketClient& client)
        {
            std::string message;

            if (!client.Receive(message))
            {
                return;
            }

            std::string_view symbol;
            double bidPrice = 0.0;
            double askPrice = 0.0;

            if (!ParseBookTicker(
                message,
                symbol,
                bidPrice,
                askPrice))
            {
                return;
            }

            Ticker ticker;

            ticker.symbol =
                std::string(symbol);

            ticker.bidPrice =
                bidPrice;

            ticker.askPrice =
                askPrice;

            cache.Update(
                ticker);
        };

    processClient(client1);
    processClient(client2);
    processClient(client3);
    processClient(client4);
    processClient(client5);
    processClient(client6);
    processClient(client7);
    processClient(client8);
}

MarketData WebSocketMarketDataProvider::GetSnapshot()
{
    return cache.GetSnapshot();
}

WebSocketIntervalStats
WebSocketMarketDataProvider::TakeIntervalStats()
{
    WebSocketIntervalStats result;

    for (auto& slotPointer :
        workerStatsSlots)
    {
        WorkerStatsSlot& slot =
            *slotPointer;

        std::lock_guard<std::mutex> lock(
            slot.mutex);

        result.Merge(
            slot.pending);

        slot.pending.Clear();
    }

    return result;
}

void WebSocketMarketDataProvider::Stop()
{
    running.store(
        false,
        std::memory_order_release);

    client1.Disconnect();
    client2.Disconnect();
    client3.Disconnect();
    client4.Disconnect();
    client5.Disconnect();
    client6.Disconnect();
    client7.Disconnect();
    client8.Disconnect();

    if (worker1.joinable()) worker1.join();
    if (worker2.joinable()) worker2.join();
    if (worker3.joinable()) worker3.join();
    if (worker4.joinable()) worker4.join();
    if (worker5.joinable()) worker5.join();
    if (worker6.joinable()) worker6.join();
    if (worker7.joinable()) worker7.join();
    if (worker8.joinable()) worker8.join();
}

void WebSocketMarketDataProvider::SetRealtimeIndexer(
    RealtimeIndexer* indexer)
{
    realtimeIndexer = indexer;
}
