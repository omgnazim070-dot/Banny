#pragma once

#include "ISymbolRegistryProvider.h"

class BinanceSymbolRegistryProvider :
    public ISymbolRegistryProvider
{
public:
    SymbolRegistry GetSymbols() override;
};