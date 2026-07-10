#include "PaperTradeJournal.h"

void PaperTradeJournal::Add(
    const PaperTrade& trade)
{
    trades.push_back(trade);
}

const std::vector<PaperTrade>&
PaperTradeJournal::GetTrades() const
{
    return trades;
}