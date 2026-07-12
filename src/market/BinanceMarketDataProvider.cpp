#include "BinanceMarketDataProvider.h"

#include "../market/Ticker.h"

#include "../../third_party/json/json.hpp"

#include <windows.h>
#include <winhttp.h>

#include <iostream>
#include <string>

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

MarketData BinanceMarketDataProvider::GetMarketData()
{
    MarketData marketData;

    HINTERNET session =
        WinHttpOpen(
            L"Banny/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

    if (!session)
    {
        std::cout
            << "WinHttpOpen failed"
            << std::endl;

        return marketData;
    }

    HINTERNET connection =
        WinHttpConnect(
            session,
            L"api.binance.com",
            INTERNET_DEFAULT_HTTPS_PORT,
            0);

    if (!connection)
    {
        WinHttpCloseHandle(session);

        return marketData;
    }

    HINTERNET request =
        WinHttpOpenRequest(
            connection,
            L"GET",
            L"/api/v3/ticker/bookTicker",
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);

    if (!request)
    {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);

        return marketData;
    }

    BOOL result =
        WinHttpSendRequest(
            request,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            WINHTTP_NO_REQUEST_DATA,
            0,
            0,
            0);

    if (!result)
    {
        return marketData;
    }

    result =
        WinHttpReceiveResponse(
            request,
            nullptr);

    if (!result)
    {
        return marketData;
    }

    std::string responseBody;

    DWORD size = 0;

    do
    {
        size = 0;

        WinHttpQueryDataAvailable(
            request,
            &size);

        if (size == 0)
        {
            break;
        }

        std::string buffer;
        buffer.resize(size);

        DWORD downloaded = 0;

        WinHttpReadData(
            request,
            buffer.data(),
            size,
            &downloaded);

        responseBody.append(
            buffer.data(),
            downloaded);

    } while (size > 0);

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    json data =
        json::parse(responseBody);

    for (const auto& item : data)
    {
        Ticker ticker;

        ticker.symbol =
            item["symbol"].get<std::string>();

        ticker.bidPrice =
            std::stod(
                item["bidPrice"].get<std::string>());

        ticker.askPrice =
            std::stod(
                item["askPrice"].get<std::string>());

        marketData.tickers[
            ticker.symbol] = ticker;
    }

    std::cout
        << "Loaded book tickers: "
        << marketData.tickers.size()
        << std::endl;

    return marketData;
}