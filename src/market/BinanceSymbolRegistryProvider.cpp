#include "BinanceSymbolRegistryProvider.h"

#include "../binance/BinanceExchangeInfoClient.h"

SymbolRegistry
BinanceSymbolRegistryProvider::GetSymbols()
{
    BinanceExchangeInfoClient client;

    SymbolRegistry registry;

    registry.symbols =
        client.GetSymbols();

    return registry;
}