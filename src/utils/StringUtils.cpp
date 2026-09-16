#include "utils/StringUtils.hpp"
#include <cctype>

bool splitOnce(const std::string& text, char separator, std::string& left, std::string& right)
{
	size_t pos = text.find(separator);
	if (pos == std::string::npos)
		return (false);
	left = text.substr(0, pos);
	right = text.substr(pos + 1);
	return (true);
}

std::string trim(const std::string& value)
{
    size_t start = value.find_first_not_of(" \t");
    if (start == std::string::npos)
        return ("");
    return (value.substr(start, value.find_last_not_of(" \t") - start + 1));
}


std::string lowercase(std::string value)
{
    for (size_t i = 0; i < value.size(); ++i)
        value[i] = std::tolower(static_cast<unsigned char>(value[i]));
    return (value);
}


int hexValue(char c)
{
	if (c >= '0' && c <= '9') return (c - '0');
	if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
	return (-1);
}

bool isToken(const std::string& value)
{
	if (value.empty()) return (false);
	for (size_t i = 0; i < value.size(); ++i)
	{
		unsigned char c = value[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || std::string("!#$%&'*+-.^_`|~").find(c) != std::string::npos)) return (false);
	}
	return (true);
}
