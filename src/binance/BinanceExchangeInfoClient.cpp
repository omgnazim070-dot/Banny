#include "BinanceExchangeInfoClient.h"

#include "../../third_party/json/json.hpp"

#include <windows.h>
#include <winhttp.h>

#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

std::vector<TradingPair>
BinanceExchangeInfoClient::GetSymbols()
{
    std::vector<TradingPair> pairs;

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

        return pairs;
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

        std::cout
            << "WinHttpConnect failed"
            << std::endl;

        return pairs;
    }

    HINTERNET request =
        WinHttpOpenRequest(
            connection,
            L"GET",
            L"/api/v3/exchangeInfo",
            nullptr,
            WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);

    if (!request)
    {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);

        std::cout
            << "WinHttpOpenRequest failed"
            << std::endl;

        return pairs;
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
        std::cout
            << "WinHttpSendRequest failed"
            << std::endl;

        return pairs;
    }

    result =
        WinHttpReceiveResponse(
            request,
            nullptr);

    if (!result)
    {
        std::cout
            << "WinHttpReceiveResponse failed"
            << std::endl;

        return pairs;
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

    for (const auto& item : data["symbols"])
    {
        if (item["status"] != "TRADING")
        {
            continue;
        }

        TradingPair pair;

        pair.symbol =
            item["symbol"].get<std::string>();

        pair.baseAsset =
            item["baseAsset"].get<std::string>();

        pair.quoteAsset =
            item["quoteAsset"].get<std::string>();

        auto isAscii =
            [](const std::string& text)
            {
                for (unsigned char c : text)
                {
                    if (c > 127)
                    {
                        return false;
                    }
                }

                return true;
            };

        if (!isAscii(pair.symbol) ||
            !isAscii(pair.baseAsset) ||
            !isAscii(pair.quoteAsset))
        {
            continue;
        }

        pairs.push_back(pair);
    }

    std::cout
        << "Loaded pairs: "
        << pairs.size()
        << std::endl;

    return pairs;
}