#include "BinanceRestClient.h"

#include "../../third_party/httplib/httplib.h"
#include "../../third_party/json/json.hpp"

#include <iostream>

using json = nlohmann::json;

MarketData BinanceRestClient::DownloadPrices()
{
    MarketData data;

    httplib::SSLClient client("api.binance.com");

    client.set_follow_location(true);

    auto response =
        client.Get("/api/v3/ticker/price");

    if (!response)
    {
        std::cout
            << "Binance request failed"
            << std::endl;

        return data;
    }

    if (response->status != 200)
    {
        std::cout
            << "HTTP Error: "
            << response->status
            << std::endl;

        return data;
    }

    auto parsed =
        json::parse(response->body);

    for (const auto& item : parsed)
    {
        std::string symbol =
            item["symbol"];

        double price =
            std::stod(
                item["price"]
            );

        data.prices[symbol] = price;
    }

    std::cout
        << "Downloaded "
        << data.prices.size()
        << " prices from Binance"
        << std::endl;

    return data;
}