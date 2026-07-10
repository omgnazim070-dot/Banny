#pragma once

#include <string>

class Logger
{
public:
    bool Start();
    void Info(const std::string& message);

private:
    std::string m_logFile = "banny.log";
};