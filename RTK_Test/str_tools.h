#ifndef STR_TOOLS_H
#define STR_TOOLS_H

#include <string>
#include <vector>

std::string Fstring(const char *format, ...);
std::vector<std::string> SpliteString(const std::string &data, const std::string &pattern);

#endif