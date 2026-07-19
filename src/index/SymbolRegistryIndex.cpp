#include "SymbolRegistryIndex.h"

#include <limits>

SymbolId SymbolRegistryIndex::Register(
    const std::string& symbol)
{
    auto it =
        ids.find(symbol);

    if (it != ids.end())
    {
        return it->second;
    }

    SymbolId id =
        static_cast<SymbolId>(
            names.size());

    ids[symbol] = id;

    names.push_back(symbol);

    return id;
}

SymbolId SymbolRegistryIndex::GetId(
    const std::string& symbol) const
{
    auto it = ids.find(symbol);

    if (it == ids.end())
    {
        return std::numeric_limits<SymbolId>::max();
    }

    return it->second;
}

const std::string&
SymbolRegistryIndex::GetName(
    SymbolId id) const
{
    return names[id];
}

size_t
SymbolRegistryIndex::Size() const
{
    return names.size();
}