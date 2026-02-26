#include "AConfig.hpp"

AConfig::ErrorException::ErrorException(std::string const& message) : _message("CONFIG ERROR: " + message) {}

AConfig::ErrorException::~ErrorException() throw() {}

const char* AConfig::ErrorException::what() const throw()
{
    return _message.c_str();
}