#include "BinanceSymbolRegistryProvider.h"

#include "../binance/BinanceExchangeInfoClient.h"

SymbolRegistry
BinanceSymbolRegistryProvider::GetSymbols()
{
    BinanceExchangeInfoClient client;

    SymbolRegistry registry;

    registry.pairs =
        client.GetSymbols();

    return registry;
}