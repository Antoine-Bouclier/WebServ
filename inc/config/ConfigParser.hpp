#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "WebServ.hpp"
#include <exception>

class ConfigParser
{
	private:
		
	public:
		ConfigParser();
		~ConfigParser();

	public:
	class ErrorException : public std::exception
	{
		private:
			std::string	_message;
		public:
			ErrorException(std::string message) throw();
			virtual const char* what() const throw();
			virtual ~ErrorException() throw();
	};
};

#endif