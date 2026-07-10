#include "TestSymbolRegistryProvider.h"

SymbolRegistry
TestSymbolRegistryProvider::GetSymbols()
{
    SymbolRegistry registry;

    registry.symbols =
    {
        "BTCUSDT",
        "ETHUSDT",
        "ETHBTC",
        "BNBUSDT",
        "BNBBTC"
    };

    return registry;
}