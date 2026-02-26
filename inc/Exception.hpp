#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <exception>
#include <iostream>

class ErrorException : public std::exception
	{
		private:
			std::string	_message;
		public:
			ErrorException(std::string const& message) : _message("CONFIG ERROR: " + message) {}
			virtual const char* what() const throw() { return _message.c_str(); }
			virtual ~ErrorException() throw() {}
	};

#endif