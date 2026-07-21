#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "SymbolId.h"

class SymbolRegistryIndex
{
public:

    SymbolId Register(
        const std::string& symbol);

    SymbolId GetId(
        std::string_view symbol) const;

    const std::string& GetName(
        SymbolId id) const;

    std::size_t Size() const;

private:

    struct TransparentStringHash
    {
        using is_transparent = void;

        std::size_t operator()(
            std::string_view value) const noexcept;
    };

    std::unordered_map<
        std::string,
        SymbolId,
        TransparentStringHash,
        std::equal_to<>> ids;

    std::vector<std::string> names;
};
