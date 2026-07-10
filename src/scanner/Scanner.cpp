#include "Scanner.h"

#include <iostream>

void Scanner::Scan()
{
    std::cout
        << "Scanning market..."
        << std::endl;

    int symbols = 1250;
    int routes = 3500;

    double bestProfit = 0.87;

    std::cout
        << "Found "
        << symbols
        << " symbols"
        << std::endl;

    std::cout
        << "Found "
        << routes
        << " triangular routes"
        << std::endl;

    std::cout
        << "Best opportunity: "
        << bestProfit
        << "%"
        << std::endl;
}