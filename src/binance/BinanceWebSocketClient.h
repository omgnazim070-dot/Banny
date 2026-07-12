#pragma once

#include <string>

#include <windows.h>
#include <winhttp.h>

class BinanceWebSocketClient
{
public:

    bool Connect(
        const std::string& url);

    void Disconnect();

    bool IsConnected() const;

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