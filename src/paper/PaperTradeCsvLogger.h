#pragma once

#include "PaperTrade.h"

class PaperTradeCsvLogger
{
public:
    bool Append(
        const PaperTrade& trade);
};