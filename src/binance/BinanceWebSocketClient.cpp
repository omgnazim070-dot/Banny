#include "BinanceWebSocketClient.h"

#include <iostream>

bool BinanceWebSocketClient::Connect(
    const std::vector<std::string>& symbols)
{
    host = L"stream.binance.com";

    std::string streamPath =
        "/stream?streams=";

    for (size_t i = 0;
        i < symbols.size();
        ++i)
    {
        std::string symbol =
            symbols[i];

        for (char& c : symbol)
        {
            c = static_cast<char>(
                std::tolower(
                    static_cast<unsigned char>(c)));
        }

        streamPath +=
            symbol +
            "@bookTicker";

        if (i + 1 < symbols.size())
        {
            streamPath += "/";
        }
    }

    path =
        std::wstring(
            streamPath.begin(),
            streamPath.end());

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

    std::cout
        << "Streams count: "
        << symbols.size()
        << std::endl;

    std::cout
        << "Path length: "
        << streamPath.size()
        << std::endl;

    request = WinHttpOpenRequest(
        connection,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);

    if (request == nullptr)
    {
        std::cout
            << "WinHttpOpenRequest failed"
            << std::endl;

        return false;
    }

    std::cout
        << "WebSocket request created"
        << std::endl;

    if (!WinHttpSetOption(
        request,
        WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
        nullptr,
        0))
    {
        std::cout
            << "WinHttpSetOption failed"
            << std::endl;

        return false;
    }

    std::cout
        << "WebSocket upgrade enabled"
        << std::endl;

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

        return false;
    }

    std::cout
        << "Upgrade request sent"
        << std::endl;

    result =
        WinHttpReceiveResponse(
            request,
            nullptr);

    if (!result)
    {
        std::cout
            << "WinHttpReceiveResponse failed"
            << std::endl;

        return false;
    }

    std::cout
        << "Upgrade response received"
        << std::endl;

    webSocket =
        WinHttpWebSocketCompleteUpgrade(
            request,
            0);

    if (webSocket == nullptr)
    {
        std::cout
            << "WebSocket upgrade failed"
            << std::endl;

        return false;
    }

    WinHttpCloseHandle(request);
    request = nullptr;

    std::cout
        << "WebSocket connected"
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

bool BinanceWebSocketClient::Send(
    const std::string& message)
{
    if (!webSocket)
    {
        return false;
    }

    DWORD result =
        WinHttpWebSocketSend(
            webSocket,
            WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
            (PVOID)message.data(),
            static_cast<DWORD>(message.size()));

    if (result != NO_ERROR)
    {
        std::cout
            << std::endl
            << "===================="
            << std::endl
            << "RECEIVE ERROR"
            << std::endl
            << "CODE: "
            << result
            << std::endl
            << "===================="
            << std::endl;

        return false;
    }

    std::cout
        << "WebSocket message sent"
        << std::endl;

    return true;
}

void BinanceWebSocketClient::Disconnect()
{

    if (webSocket)
    {
        WinHttpWebSocketClose(
            webSocket,
            WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
            nullptr,
            0);

        WinHttpCloseHandle(webSocket);

        webSocket = nullptr;
    }

    if (request)
    {
        WinHttpCloseHandle(request);
        request = nullptr;
    }

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

bool BinanceWebSocketClient::Receive(
    std::string& message)
{
    if (!webSocket)
    {
        return false;
    }

    char buffer[65536];

    DWORD bytesRead = 0;

    WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;

    DWORD result =
        WinHttpWebSocketReceive(
            webSocket,
            buffer,
            sizeof(buffer),
            &bytesRead,
            &bufferType);

    if (result != NO_ERROR)
    {
        if (result != 4317)
        {
            std::cout
                << "Receive failed: "
                << result
                << std::endl;
        }

        return false;
    }

    message.assign(
        buffer,
        bytesRead);

    return true;
}

bool BinanceWebSocketClient::IsConnected() const
{
    return connected;
}