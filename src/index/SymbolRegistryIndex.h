#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "SymbolId.h"

class SymbolRegistryIndex
{
public:

    SymbolId Register(
        const std::string& symbol);

    SymbolId GetId(
        const std::string& symbol) const;

    const std::string& GetName(
        SymbolId id) const;

    size_t Size() const;

private:

    std::unordered_map<
        std::string,
        SymbolId> ids;

    std::vector<std::string> names;
};