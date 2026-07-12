#pragma once

#include "SymbolRegistry.h"

class ISymbolRegistryProvider
{
public:
    virtual ~ISymbolRegistryProvider() = default;

    virtual SymbolRegistry GetSymbols() = 0;
};