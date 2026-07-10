#pragma once

#include <vector>

#include "PaperTrade.h"

class PaperTradeJournal
{
public:
    void Add(
        const PaperTrade& trade);

    const std::vector<PaperTrade>&
        GetTrades() const;

private:
    std::vector<PaperTrade> trades;
};