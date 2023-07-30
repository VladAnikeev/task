#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <sstream>
#include <fstream>

class Log
{
private:
    std::ofstream file;

public:
    Log();
    template <typename T>
    Log &operator<<(T message);
    ~Log();
};

/**
 * @brief создает файл log.log
 *
 */
Log::Log() : file("log.log")
{
    if (!file.is_open())
    {
        std::cout << "failed to open " << '\n';
        throw "";
    }
}
template <typename T>
Log &Log::operator<<(T message)
{
    std::cout << message;
    file << message;
    return (*this);
};

Log::~Log()
{
}

#endif
