#include <cstdarg>
#include <cstring>
#include "str_tools.h"

const size_t size = 64;

std::string Fstring(const char *format, ...)
{
    char *buff = new char[size];
    memset(buff, 0, size);
    va_list argList;
    va_start(argList, format);
    vsnprintf(buff, size, format, argList);
    va_end(argList);
    std::string ret(buff);
    delete[] buff;
    return ret;
}

std::vector<std::string> SpliteString(const std::string &data, const std::string &pattern)
{
    std::vector<std::string> result;
    std::string split_data = data + pattern;
    size_t pos = split_data.find(pattern);
    size_t size = split_data.size();
    while (pos != std::string::npos)
    {
        std::string tmp = split_data.substr(0, pos);
        result.emplace_back(tmp);
        split_data = split_data.substr(pos + 1, size);
        pos = split_data.find(pattern);
    }
    return result;
}
