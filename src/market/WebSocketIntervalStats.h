#pragma once

#include <cstdint>

#include "../core/LatencyHistogram.h"

struct WebSocketIntervalStats
{
    std::uint64_t receivedMessages = 0;

    std::uint64_t parsedMessages = 0;

    std::uint64_t parseErrors = 0;

    std::uint64_t priceChanges = 0;

    std::uint64_t unchangedPricesSkipped = 0;

    std::uint64_t unknownSymbols = 0;

    LatencyHistogram receiveToIndex;

    void Merge(
        const WebSocketIntervalStats& other)
    {
        receivedMessages +=
            other.receivedMessages;

        parsedMessages +=
            other.parsedMessages;

        parseErrors +=
            other.parseErrors;

        priceChanges +=
            other.priceChanges;

        unchangedPricesSkipped +=
            other.unchangedPricesSkipped;

        unknownSymbols +=
            other.unknownSymbols;

        receiveToIndex.Merge(
            other.receiveToIndex);
    }

    void Clear()
    {
        receivedMessages = 0;
        parsedMessages = 0;
        parseErrors = 0;
        priceChanges = 0;
        unchangedPricesSkipped = 0;
        unknownSymbols = 0;

        receiveToIndex.Clear();
    }
};
