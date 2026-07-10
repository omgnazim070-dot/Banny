#include <iostream>

#include "config/Config.h"
#include "logger/Logger.h"
#include "core/Bot.h"

int main()
{
    std::cout << "[Banny]" << std::endl;

    Config config;
    config.Load();

    Logger logger;
    logger.Start();

    Bot bot;
    bot.Run();

    return 0;
}