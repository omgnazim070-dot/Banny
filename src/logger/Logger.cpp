#include "Logger.h"

#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

static std::string GetTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
    localtime_s(&localTime, &time);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

bool Logger::Start()
{
    std::cout << "Log file: " << m_logFile << std::endl;

    std::ofstream log(m_logFile, std::ios::app);

    if (!log.is_open())
    {
        std::cout << "Failed to open log file" << std::endl;
        return false;
    }

    log << "[" << GetTimeStamp() << "] "
        << "[INFO] Logger started"
        << std::endl;
    log.close();

    return true;
}

void Logger::Info(const std::string& message)
{
    std::ofstream log(m_logFile, std::ios::app);

    if (log.is_open())
    {
        log << "[" << GetTimeStamp() << "] "
            << "[INFO] "
            << message
            << std::endl;

        log.close();
    }
}