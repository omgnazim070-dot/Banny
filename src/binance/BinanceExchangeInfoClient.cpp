#include "BinanceExchangeInfoClient.h"

#include "../../third_party/json/json.hpp"

#include <windows.h>
#include <winhttp.h>

#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

std::vector<std::string>
BinanceExchangeInfoClient::GetSymbols()
{
    std::vector<std::string> symbols;

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

        return symbols;
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

        return symbols;
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

        return symbols;
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

        return symbols;
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

        return symbols;
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

    for (const auto& symbol : data["symbols"])
    {
        if (symbol["status"] != "TRADING")
        {
            continue;
        }

        symbols.push_back(
            symbol["symbol"].get<std::string>());
    }

    std::cout
        << "Loaded symbols: "
        << symbols.size()
        << std::endl;

    return symbols;
}