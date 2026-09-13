#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <sstream>
#include <exception>

class ErrorException : public std::exception
{
	private:
		std::string	_message;

	public:
		ErrorException() : _message("CONFIG ERROR") {}
		ErrorException(const ErrorException& other) : std::exception(other), _message(other._message) {}
		ErrorException& operator=(const ErrorException& other)
		{
			if (this != &other)
			{
				std::exception::operator=(other);
				_message = other._message;
			}
			return (*this);
		}
		ErrorException(std::string const& message) : _message("CONFIG ERROR: " + message) {}
		ErrorException(std::string const& message, unsigned int line) 
		{
			std::stringstream ss;
			ss << "CONFIG ERROR (line " << line << "): " << message;
			_message = ss.str();
		}
		virtual const char* what() const throw() { return _message.c_str(); }
		virtual ~ErrorException() throw() {}
};

#endif
