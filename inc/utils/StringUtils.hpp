#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <cstddef>
#include <string>

bool splitOnce(const std::string& text, char separator, std::string& left, std::string& right);

std::string trim(const std::string& value);
std::string lowercase(std::string value);

bool parseHeaderLine(const std::string& line, std::string& name, std::string& value);

bool parseUnsignedNumber(const std::string& text, std::size_t& value);

int hexValue(char c);
bool isToken(const std::string& value);
bool isValidHeaderValue(const std::string& value);

#endif
