#pragma once

#include <queue>
#include <vector>

#include "SymbolId.h"
#include <mutex>

class DirtyQueue
{
public:

    void Mark(
        TriangleId triangle);

    bool Empty() const;

    TriangleId Pop();

    void Complete(
        TriangleId triangle);

    void Clear();

private:

    std::queue<TriangleId> queue;

    std::vector<bool> exists;

    mutable std::mutex mutex;
};