#include "BinanceWebSocketClient.h"

#include <iostream>

bool BinanceWebSocketClient::Connect(
    const std::string& url)
{
    currentUrl = url;

    host = L"stream.binance.com";
    path = L"/ws";
    port = INTERNET_DEFAULT_HTTPS_PORT;

    session = WinHttpOpen(
        L"Banny/2.5",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (session == nullptr)
    {
        std::cout
            << "WinHttpOpen failed"
            << std::endl;

        return false;
    }

    connection = WinHttpConnect(
        session,
        host.c_str(),
        port,
        0);

    if (connection == nullptr)
    {
        std::cout
            << "WinHttpConnect failed"
            << std::endl;

        return false;
    }

    std::cout
        << "Connected to Binance host"
        << std::endl;

    std::wcout
        << L"Host: "
        << host
        << std::endl;

    std::wcout
        << L"Path: "
        << path
        << std::endl;

    connected = true;

    std::cout
        << "WinHTTP session created"
        << std::endl;

    return true;
}

void BinanceWebSocketClient::Disconnect()
{
    if (connection)
    {
        WinHttpCloseHandle(connection);
        connection = nullptr;
    }

    if (session)
    {
        WinHttpCloseHandle(session);
        session = nullptr;
    }

    connected = false;

    std::cout
        << "WebSocket disconnected"
        << std::endl;
}

bool BinanceWebSocketClient::IsConnected() const
{
    return connected;
}