#include "TestSymbolRegistryProvider.h"

SymbolRegistry
TestSymbolRegistryProvider::GetSymbols()
{
    SymbolRegistry registry;

    registry.pairs.push_back(
        { "BTCUSDT", "BTC", "USDT" });

    registry.pairs.push_back(
        { "ETHUSDT", "ETH", "USDT" });

    registry.pairs.push_back(
        { "ETHBTC", "ETH", "BTC" });

    registry.pairs.push_back(
        { "BNBUSDT", "BNB", "USDT" });

    registry.pairs.push_back(
        { "BNBBTC", "BNB", "BTC" });

    return registry;
}