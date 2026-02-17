#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "WebServ.hpp"
#include "ConfigFile.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>

class ConfigParser
{
	private:
		std::string	readFile(const std::string &path);
	public:
		static ConfigFile	parseConfig(const std::string &path);

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