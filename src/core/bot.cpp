#include "Bot.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory>
#include <numeric>
#include <vector>

#include "../config/Config.h"
#include "../config/TradingSettings.h"

#include "../logger/Logger.h"

#include "../scanner/Scanner.h"
#include "../triangle/TriangleBuilder.h"
#include "../arbitrage/ArbitrageEngine.h"

#include "../market/SymbolResolver.h"
#include "../market/TestSymbolRegistryProvider.h"

#include "../market/BinanceMarketDataProvider.h"
#include "../paper/PaperTradeJournal.h"

#include "../paper/PaperTradeCsvLogger.h"

#include "../market/BinanceSymbolRegistryProvider.h"

#include <thread>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <chrono>

#include "../market/MarketMonitor.h"
#include "../binance/BinanceWebSocketClient.h"

#include "../market/MarketDataCache.h"

#include <iomanip>
#include "../market/WebSocketMarketDataProvider.h"

#include "../index/SymbolRegistryIndex.h"
#include "../index/IndexedTriangleBuilder.h"
#include "../index/TriangleIndex.h"
#include "../index/DirtyQueue.h"
#include "../index/IndexedMarketCache.h"
#include "../index/RealtimeIndexer.h"
#include "../index/TriangleRuntimeState.h"
#include "WorkerIntervalStats.h"

namespace
{
    struct WorkerPublishSlot
    {
        std::mutex mutex;

        WorkerIntervalStats pending;
    };
}

void Bot::Run()
{
    std::cout << "[Banny]" << std::endl;

    Logger logger;

    if (!logger.Start())
    {
        return;
    }

    logger.Info("Banny started");

    Config config;

    if (!config.Load())
    {
        return;
    }

    std::cout
        << "Bot Name: "
        << config.BotName
        << std::endl;

    std::cout
        << "Trading Mode: "
        << config.TradingMode
        << std::endl;

    std::cout
        << "Scan Interval: "
        << config.ScanIntervalMs
        << std::endl;

    std::cout
        << "Min Profit: "
        << config.MinProfitPercent
        << std::endl;

    logger.Info("Config loaded");

    BinanceSymbolRegistryProvider symbolProvider;

    auto registry =
        symbolProvider.GetSymbols();

    std::vector<std::string> wsSymbols;


    for (const auto& pair : registry.pairs)
    {
        wsSymbols.push_back(
            pair.symbol);
    }

    std::cout
        << "\n===== BOT SYMBOL CHECK ====="
        << std::endl;

    for (const auto& s : wsSymbols)
    {
        if (s.find_first_not_of(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos)
        {
            std::cout
                << "BOT BAD SYMBOL: "
                << s
                << std::endl;
        }
    }

    std::cout
        << "Pairs loaded: "
        << registry.pairs.size()
        << std::endl;

    if (!registry.pairs.empty())
    {
        const auto& pair =
            registry.pairs.front();

        std::cout
            << pair.symbol
            << " | "
            << pair.baseAsset
            << " | "
            << pair.quoteAsset
            << std::endl;
    }

    std::vector<std::string> symbols;

    std::cout
        << "Registry pairs: "
        << registry.pairs.size()
        << std::endl;

    for (size_t i = 0;
        i < std::min<size_t>(20, registry.pairs.size());
        ++i)
    {
        std::cout
            << registry.pairs[i].symbol
            << std::endl;;
    }

    std::cout
        << "STEP 4"
        << std::endl;

    TriangleBuilder builder;

    auto triangles =
        builder.Build(
            registry.pairs);

    SymbolRegistryIndex symbolIndex;

    IndexedTriangleBuilder indexedBuilder;

    auto indexedTriangles =
        indexedBuilder.Build(
            triangles,
            symbolIndex);

    std::cout
        << "Indexed symbols after build: "
        << symbolIndex.Size()
        << std::endl;

    TriangleIndex triangleIndex;

    triangleIndex.Build(
        indexedTriangles,
        symbolIndex.Size());

    IndexedMarketCache indexedCache;

    indexedCache.Resize(
        symbolIndex.Size());

    unsigned int hardwareThreads =
        std::thread::hardware_concurrency();

    if (hardwareThreads == 0)
    {
        hardwareThreads = 2;
    }

    std::size_t engineCount =
        hardwareThreads / 2;

    engineCount = std::clamp<std::size_t>(
        engineCount,
        1,
        8);

    std::vector<
        std::unique_ptr<DirtyQueue>>
        engineQueues;

    std::vector<DirtyQueue*>
        engineQueuePointers;

    engineQueues.reserve(
        engineCount);

    engineQueuePointers.reserve(
        engineCount);

    for (std::size_t engineId = 0;
        engineId < engineCount;
        ++engineId)
    {
        engineQueues.push_back(
            std::make_unique<DirtyQueue>(
                indexedTriangles.size()));

        engineQueuePointers.push_back(
            engineQueues.back().get());
    }

    std::vector<TriangleRuntimeState>
        triangleRuntimeStates(
            indexedTriangles.size());

    RealtimeIndexer realtimeIndexer(
        symbolIndex,
        triangleIndex,
        engineQueuePointers,
        indexedCache);

    WebSocketMarketDataProvider wsProvider;

    wsProvider.SetRealtimeIndexer(
        &realtimeIndexer);

    if (!wsProvider.Start(
        wsSymbols))
    {
        std::cout
            << "STEP 1"
            << std::endl;

        std::cout
            << "WebSocket provider start failed"
            << std::endl;

        return;
    }

    std::cout
        << "STEP 2"
        << std::endl;

    std::this_thread::sleep_for(
        std::chrono::seconds(3));

    std::cout
        << "STEP 3"
        << std::endl;

    std::cout
        << "Initial market warmup completed"
        << std::endl;

    for (auto& queue :
        engineQueues)
    {
        queue->Clear();
    }

    const std::int64_t initialEventTimeNs =
        std::chrono::duration_cast<
        std::chrono::nanoseconds>(
            std::chrono::steady_clock::now()
            .time_since_epoch())
        .count();

    for (std::size_t triangle = 0;
        triangle < indexedTriangles.size();
        ++triangle)
    {
        const std::size_t engineId =
            triangle % engineCount;

        engineQueues[engineId]->Mark(
            static_cast<TriangleId>(
                triangle),
            initialEventTimeNs);
    }

    std::cout
        << "Engine queues initialized: "
        << engineCount
        << std::endl;

    std::cout
        << "Indexed symbols: "
        << symbolIndex.Size()
        << std::endl;

    std::cout
        << "Indexed triangles: "
        << indexedTriangles.size()
        << std::endl;

    std::cout
        << std::endl
        << "Found "
        << triangles.size()
        << " triangles"
        << std::endl
        << std::endl;

    TradingSettings settings;

    settings.minProfitPercent =
        config.MinProfitPercent;

    settings.commissionPercent =
        config.CommissionPercent;

    settings.slippagePercent =
        config.SlippagePercent;

    settings.startBalance =
        config.StartBalance;

    PaperTradeJournal journal;

    PaperTradeCsvLogger csvLogger;

    constexpr std::size_t maxBatchSize =
        256;

    constexpr auto reportInterval =
        std::chrono::milliseconds(
            500);

    constexpr auto statsPublishInterval =
        std::chrono::milliseconds(
            100);

    std::cout
        << "Analysis engines: "
        << engineCount
        << std::endl;

    std::vector<
        std::unique_ptr<WorkerPublishSlot>>
        workerStatsSlots;

    workerStatsSlots.reserve(
        engineCount);

    for (std::size_t engineId = 0;
        engineId < engineCount;
        ++engineId)
    {
        workerStatsSlots.push_back(
            std::make_unique<WorkerPublishSlot>());
    }

    std::vector<std::thread>
        analysisWorkers;

    analysisWorkers.reserve(
        engineCount);

    for (std::size_t engineId = 0;
        engineId < engineCount;
        ++engineId)
    {
        analysisWorkers.emplace_back(
            [&, engineId]()
            {
                MarketMonitor monitor;

                DirtyQueue& queue =
                    *engineQueues[engineId];

                WorkerPublishSlot& publishSlot =
                    *workerStatsSlots[engineId];

                WorkerIntervalStats localStats;

                std::vector<DirtyTask> tasks;

                tasks.reserve(
                    maxBatchSize);

                auto nextPublishTime =
                    std::chrono::steady_clock::now() +
                    statsPublishInterval;

                bool spinBeforeWait = false;

                while (true)
                {
                    std::uint64_t spinWaitUs = 0;

                    const bool hasTasks =
                        queue.WaitAndDrain(
                            tasks,
                            maxBatchSize,
                            nextPublishTime,
                            spinBeforeWait,
                            spinWaitUs);

                    localStats.spinWaitUs +=
                        spinWaitUs;

                    spinBeforeWait =
                        hasTasks;

                    auto currentTime =
                        std::chrono::steady_clock::now();

                    if (hasTasks &&
                        !tasks.empty())
                    {
                        const auto analysisStart =
                            currentTime;

                        monitor.RunBatch(
                            indexedTriangles,
                            tasks,
                            queue,
                            indexedCache,
                            triangleRuntimeStates,
                            settings,
                            localStats);

                        currentTime =
                            std::chrono::steady_clock::now();

                        const long long analysisUs =
                            std::chrono::duration_cast<
                            std::chrono::microseconds>(
                                currentTime -
                                analysisStart)
                            .count();

                        localStats.batches++;

                        if (analysisUs > 0)
                        {
                            localStats.engineWorkUs +=
                                static_cast<std::uint64_t>(
                                    analysisUs);
                        }
                    }

                    if (currentTime < nextPublishTime)
                    {
                        continue;
                    }

                    if (localStats.batches > 0)
                    {
                        std::lock_guard<std::mutex> lock(
                            publishSlot.mutex);

                        publishSlot.pending.Merge(
                            localStats);

                        localStats.Clear();
                    }

                    do
                    {
                        nextPublishTime +=
                            statsPublishInterval;
                    }
                    while (currentTime >=
                        nextPublishTime);
                }
            });
    }

    auto lastReportTime =
        std::chrono::steady_clock::now();

    while (true)
    {
        std::this_thread::sleep_for(
            reportInterval);

        const auto reportTime =
            std::chrono::steady_clock::now();

        const std::int64_t reportTimeNs =
            std::chrono::duration_cast<
            std::chrono::nanoseconds>(
                reportTime.time_since_epoch())
            .count();

        const long long reportWindowUs =
            std::chrono::duration_cast<
            std::chrono::microseconds>(
                reportTime -
                lastReportTime)
            .count();

        lastReportTime =
            reportTime;

        WorkerIntervalStats reportStats;

        std::vector<std::uint64_t>
            reportEngineWorkUs(
                engineCount,
                0);

        for (std::size_t engineId = 0;
            engineId < engineCount;
            ++engineId)
        {
            WorkerPublishSlot& publishSlot =
                *workerStatsSlots[engineId];

            std::lock_guard<std::mutex> lock(
                publishSlot.mutex);

            reportEngineWorkUs[engineId] =
                publishSlot.pending.engineWorkUs +
                publishSlot.pending.spinWaitUs;

            reportStats.Merge(
                publishSlot.pending);

            publishSlot.pending.Clear();
        }

        std::uint64_t queueAccepted = 0;
        std::uint64_t queueCoalesced = 0;
        std::uint64_t immediateRechecks = 0;
        std::uint64_t deferredRechecks = 0;

        std::size_t queueDepth = 0;
        std::size_t queuePeak = 0;

        std::size_t busiestEngineId = 0;
        std::size_t busiestQueueDepth = 0;

        std::uint64_t oldestQueuedTaskAgeUs = 0;

        for (std::size_t engineId = 0;
            engineId < engineCount;
            ++engineId)
        {
            DirtyQueue& queue =
                *engineQueues[engineId];

            queueAccepted +=
                queue.TakeAcceptedMarks();

            queueCoalesced +=
                queue.TakeCoalescedMarks();

            immediateRechecks +=
                queue.TakeImmediateRechecks();

            deferredRechecks +=
                queue.TakeDeferredRechecks();

            const std::size_t currentDepth =
                queue.Size();

            queueDepth +=
                currentDepth;

            queuePeak +=
                queue.TakePeakDepth();

            oldestQueuedTaskAgeUs =
                (std::max)(
                    oldestQueuedTaskAgeUs,
                    queue.OldestQueuedAgeUs(
                        reportTimeNs));

            if (currentDepth >
                busiestQueueDepth)
            {
                busiestQueueDepth =
                    currentDepth;

                busiestEngineId =
                    engineId;
            }
        }

        const double averageBatchSize =
            reportStats.batches > 0
            ? static_cast<double>(
                reportStats.processedTasks) /
            static_cast<double>(
                reportStats.batches)
            : 0.0;

        const double engineLoadPercent =
            engineCount > 0 &&
            reportWindowUs > 0
            ? static_cast<double>(
                reportStats.engineWorkUs +
                reportStats.spinWaitUs) *
            100.0 /
            (static_cast<double>(
                engineCount) *
                static_cast<double>(
                    reportWindowUs))
            : 0.0;

        std::size_t busiestWorkEngine = 0;
        std::uint64_t maximumEngineWorkUs = 0;

        for (std::size_t engineId = 0;
            engineId <
            reportEngineWorkUs.size();
            ++engineId)
        {
            if (reportEngineWorkUs[engineId] >
                maximumEngineWorkUs)
            {
                maximumEngineWorkUs =
                    reportEngineWorkUs[engineId];

                busiestWorkEngine =
                    engineId;
            }
        }

        const double maximumEngineLoad =
            reportWindowUs > 0
            ? static_cast<double>(
                maximumEngineWorkUs) *
            100.0 /
            static_cast<double>(
                reportWindowUs)
            : 0.0;

        std::string bestRoute;

        if (reportStats.hasBest &&
            reportStats.bestTriangleId <
            indexedTriangles.size())
        {
            const auto& triangle =
                indexedTriangles[
                    reportStats.bestTriangleId];

            bestRoute =
                symbolIndex.GetName(
                    triangle.pairAB)
                + " -> " +
                symbolIndex.GetName(
                    triangle.pairBC)
                + " -> " +
                symbolIndex.GetName(
                    triangle.pairCA);
        }

        const WebSocketIntervalStats webSocketStats =
            wsProvider.TakeIntervalStats();

        const std::uint64_t tickerUpdates =
            realtimeIndexer.TakeTickerUpdates();

        const std::uint64_t priceChanges =
            realtimeIndexer.TakePriceChanges();

        const std::uint64_t unchangedPrices =
            realtimeIndexer.TakeUnchangedPrices();

        const std::uint64_t unknownSymbols =
            realtimeIndexer.TakeUnknownSymbols();

        const std::uint64_t disconnectEvents =
            realtimeIndexer.TakeDisconnectEvents();

        const std::uint64_t invalidatedSymbols =
            realtimeIndexer.TakeInvalidatedSymbols();

        std::cout
            << '\n'
            << "=== BANNY SHARDED ENGINES ===\n"
            << "Analysis engines: "
            << engineCount
            << '\n'
            << "Independent queues: "
            << engineQueues.size()
            << '\n'
            << "Subscribed symbols: "
            << wsSymbols.size()
            << '\n'
            << "Indexed symbols: "
            << symbolIndex.Size()
            << '\n'
            << "Total triangles: "
            << indexedTriangles.size()
            << '\n'
            << '\n'
            << "WebSocket messages / interval: "
            << webSocketStats.receivedMessages
            << '\n'
            << "Parsed messages / interval: "
            << webSocketStats.parsedMessages
            << '\n'
            << "Parse errors / interval: "
            << webSocketStats.parseErrors
            << '\n'
            << "Ticker updates / interval: "
            << tickerUpdates
            << '\n'
            << "Price changes / interval: "
            << priceChanges
            << '\n'
            << "Unchanged prices skipped: "
            << unchangedPrices
            << '\n'
            << "Receive-to-index average: "
            << webSocketStats.receiveToIndex.AverageUs()
            << " us\n"
            << "Receive-to-index P95: "
            << webSocketStats.receiveToIndex.PercentileUs(
                0.95)
            << " us\n"
            << "Receive-to-index P99: "
            << webSocketStats.receiveToIndex.PercentileUs(
                0.99)
            << " us\n"
            << "Receive-to-index maximum: "
            << webSocketStats.receiveToIndex.MaximumUs()
            << " us\n"
            << "Queue accepted / interval: "
            << queueAccepted
            << '\n'
            << "Queue coalesced / interval: "
            << queueCoalesced
            << '\n'
            << "Immediate rechecks / interval: "
            << immediateRechecks
            << '\n'
            << "Deferred rechecks / interval: "
            << deferredRechecks
            << '\n'
            << "Queue depth total: "
            << queueDepth
            << '\n'
            << "Queue peak total: "
            << queuePeak
            << '\n'
            << "Oldest queued task age: "
            << oldestQueuedTaskAgeUs
            << " us\n"
            << "Busiest queue: "
            << busiestEngineId
            << " | depth: "
            << busiestQueueDepth
            << '\n'
            << '\n'
            << "Batches / interval: "
            << reportStats.batches
            << '\n'
            << "Average batch size: "
            << averageBatchSize
            << '\n'
            << "Tasks processed / interval: "
            << reportStats.processedTasks
            << '\n'
            << "Analyzed / interval: "
            << reportStats.analyzedTriangles
            << '\n'
            << "Average engine load: "
            << engineLoadPercent
            << "%\n"
            << "Maximum engine load: "
            << maximumEngineLoad
            << "% | engine: "
            << busiestWorkEngine
            << '\n'
            << "Total analysis work: "
            << reportStats.engineWorkUs
            << " us\n"
            << "Total active spin: "
            << reportStats.spinWaitUs
            << " us\n"
            << '\n'
            << "Queue wait samples: "
            << reportStats.queueWait.Count()
            << '\n'
            << "Queue wait average: "
            << reportStats.queueWait.AverageUs()
            << " us\n"
            << "Queue wait P95: "
            << reportStats.queueWait.PercentileUs(
                0.95)
            << " us\n"
            << "Queue wait P99: "
            << reportStats.queueWait.PercentileUs(
                0.99)
            << " us\n"
            << "Queue wait maximum: "
            << reportStats.queueWait.MaximumUs()
            << " us\n"
            << '\n'
            << "Calculation average: "
            << reportStats.calculation.AverageUs()
            << " us\n"
            << "Calculation P95: "
            << reportStats.calculation.PercentileUs(
                0.95)
            << " us\n"
            << "Calculation P99: "
            << reportStats.calculation.PercentileUs(
                0.99)
            << " us\n"
            << "Calculation maximum: "
            << reportStats.calculation.MaximumUs()
            << " us\n"
            << '\n'
            << "End-to-end latency average: "
            << reportStats.totalLatency.AverageUs()
            << " us\n"
            << "End-to-end latency P95: "
            << reportStats.totalLatency.PercentileUs(
                0.95)
            << " us\n"
            << "End-to-end latency P99: "
            << reportStats.totalLatency.PercentileUs(
                0.99)
            << " us\n"
            << "End-to-end latency maximum: "
            << reportStats.totalLatency.MaximumUs()
            << " us\n"
            << '\n'
            << "Missing ticker / interval: "
            << reportStats.missingTickerTriangles
            << '\n'
            << "Stale / interval: "
            << reportStats.staleTriangles
            << '\n'
            << "Duplicate versions skipped: "
            << reportStats.duplicateVersionSkips
            << '\n'
            << "Disconnects / interval: "
            << disconnectEvents
            << '\n'
            << "Invalidated symbols: "
            << invalidatedSymbols
            << '\n'
            << "Unindexed ticker updates: "
            << unknownSymbols
            << '\n'
            << '\n'
            << "Profitable / interval: "
            << reportStats.profitableTriangles
            << '\n'
            << "Best profit: "
            << (reportStats.hasBest
                ? reportStats.bestProfitPercent
                : 0.0)
            << "%\n"
            << "Best route: "
            << bestRoute
            << std::endl;
    }

    logger.Info("Core initialized");

}
