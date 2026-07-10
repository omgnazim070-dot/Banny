#include "SymbolResolver.h"

TrianglePairs SymbolResolver::Resolve(
    const std::string& assetA,
    const std::string& assetB,
    const std::string& assetC)
{
    TrianglePairs result;

    result.pair1 = assetB + assetA;
    result.pair2 = assetC + assetB;
    result.pair3 = assetC + assetA;

    return result;
}