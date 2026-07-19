#pragma once

#include <vector>

#include "TriangleIndex.h"
#include "SymbolRegistryIndex.h"

#include "../triangle/TriangleBuilder.h"

class IndexedTriangleBuilder
{
public:

    std::vector<IndexedTriangle> Build(
        const std::vector<Triangle>& triangles,
        SymbolRegistryIndex& registry);
};