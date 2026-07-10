#pragma once

#include "../scanner/MarketData.h"

class BinanceRestClient
{
public:
    MarketData DownloadPrices();
};