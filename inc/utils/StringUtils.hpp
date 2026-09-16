#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <string>

bool splitOnce(const std::string& text, char separator, std::string& left, std::string& right);

std::string trim(const std::string& value);
std::string lowercase(std::string value);

int hexValue(char c);
bool isToken(const std::string& value);

#endif
