#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "WebServ.hpp"
#include "Lexer.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>

class ConfigParser
{
	private:
		std::string	_path;
		Lexer		_lexer;

		void		checkExt();
		std::string	readFile(const char* path);
	public:
		/* Main function */
		void	parseConfig(const char* path);

	public:
	class ErrorException : public std::exception
	{
		private:
			std::string	_message;
		public:
			ErrorException(std::string const& message);
			virtual const char* what() const throw();
			virtual ~ErrorException() throw();
	};
};

#endif