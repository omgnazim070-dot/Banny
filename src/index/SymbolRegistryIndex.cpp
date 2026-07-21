#include "SymbolRegistryIndex.h"

#include <cstdint>
#include <limits>

std::size_t
SymbolRegistryIndex::TransparentStringHash::operator()(
    std::string_view value) const noexcept
{
#if SIZE_MAX == UINT64_MAX
    std::size_t hash =
        static_cast<std::size_t>(1469598103934665603ULL);

    constexpr std::size_t prime =
        static_cast<std::size_t>(1099511628211ULL);
#else
    std::size_t hash =
        static_cast<std::size_t>(2166136261U);

    constexpr std::size_t prime =
        static_cast<std::size_t>(16777619U);
#endif

    for (const unsigned char character :
        value)
    {
        hash ^=
            static_cast<std::size_t>(character);

        hash *= prime;
    }

    return hash;
}

SymbolId SymbolRegistryIndex::Register(
    const std::string& symbol)
{
    auto it =
        ids.find(symbol);

    if (it != ids.end())
    {
        return it->second;
    }

    const SymbolId id =
        static_cast<SymbolId>(
            names.size());

    ids.emplace(
        symbol,
        id);

    names.push_back(
        symbol);

    return id;
}

SymbolId SymbolRegistryIndex::GetId(
    std::string_view symbol) const
{
    const auto it =
        ids.find(symbol);

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

std::size_t
SymbolRegistryIndex::Size() const
{
    return names.size();
}
