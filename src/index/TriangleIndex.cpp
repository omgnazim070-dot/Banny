#include "TriangleIndex.h"

#include <algorithm>
#include <iostream>


void TriangleIndex::Build(
    const std::vector<IndexedTriangle>& triangles,
    size_t symbolCount)
{
    index.clear();

    size_t maxSymbol = symbolCount;


    for (const auto& t : triangles)
    {
        maxSymbol = std::max(
            maxSymbol,
            static_cast<size_t>(t.pairAB) + 1);

        maxSymbol = std::max(
            maxSymbol,
            static_cast<size_t>(t.pairBC) + 1);

        maxSymbol = std::max(
            maxSymbol,
            static_cast<size_t>(t.pairCA) + 1);
    }


    std::cout
        << "FINAL INDEX SIZE: "
        << maxSymbol
        << std::endl;


    index.resize(maxSymbol);


    for (size_t id = 0;
        id < triangles.size();
        ++id)
    {
        const auto& t =
            triangles[id];


        size_t ab =
            static_cast<size_t>(t.pairAB);

        size_t bc =
            static_cast<size_t>(t.pairBC);

        size_t ca =
            static_cast<size_t>(t.pairCA);


        if (ab >= index.size() ||
            bc >= index.size() ||
            ca >= index.size())
        {
            std::cout
                << "BAD TRIANGLE "
                << id
                << " IDS: "
                << ab
                << " "
                << bc
                << " "
                << ca
                << " INDEX SIZE: "
                << index.size()
                << std::endl;

            continue;
        }


        index[ab].push_back(
            static_cast<TriangleId>(id));

        index[bc].push_back(
            static_cast<TriangleId>(id));

        index[ca].push_back(
            static_cast<TriangleId>(id));
    }
}


const std::vector<TriangleId>&
TriangleIndex::GetTriangles(
    SymbolId symbol) const
{
    return index[symbol];
}