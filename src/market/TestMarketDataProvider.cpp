#include "TestMarketDataProvider.h"

#include "../market/Ticker.h"

MarketData TestMarketDataProvider::GetMarketData()
{
    MarketData data;

    Ticker btc;

    btc.symbol = "BTCUSDT";
    btc.bidPrice = 100000.0;
    btc.askPrice = 100010.0;

    data.tickers[btc.symbol] = btc;

    Ticker ethBtc;

    ethBtc.symbol = "ETHBTC";
    ethBtc.bidPrice = 0.03;
    ethBtc.askPrice = 0.0301;

    data.tickers[ethBtc.symbol] = ethBtc;

    Ticker ethUsdt;

    ethUsdt.symbol = "ETHUSDT";
    ethUsdt.bidPrice = 3200.0;
    ethUsdt.askPrice = 3205.0;

    data.tickers[ethUsdt.symbol] = ethUsdt;

    Ticker bnbBtc;

    bnbBtc.symbol = "BNBBTC";
    bnbBtc.bidPrice = 0.006;
    bnbBtc.askPrice = 0.0061;

    data.tickers[bnbBtc.symbol] = bnbBtc;

    Ticker bnbUsdt;

    bnbUsdt.symbol = "BNBUSDT";
    bnbUsdt.bidPrice = 620.0;
    bnbUsdt.askPrice = 621.0;

    data.tickers[bnbUsdt.symbol] = bnbUsdt;

    return data;
}