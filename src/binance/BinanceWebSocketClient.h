#pragma once

#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <winhttp.h>

class BinanceWebSocketClient
{
public:

    bool Connect(
        const std::vector<std::string>& symbols);

    bool Receive(
        std::string& message);

    void Disconnect();

    bool IsConnected() const;

    bool Send(
        const std::string& message);

private:

    bool connected = false;

    std::string currentUrl;

    std::wstring host;

    std::wstring path;

    INTERNET_PORT port =
        INTERNET_DEFAULT_HTTPS_PORT;

    HINTERNET session = nullptr;

    HINTERNET connection = nullptr;

    HINTERNET request = nullptr;

    HINTERNET webSocket = nullptr;
};