#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "WebServ.hpp"
#include "Lexer.hpp"
#include "ConfigServer.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>

class ConfigParser
{
	private:
		std::string					_path;
		Lexer						_lexer;
		std::vector<ConfigServer>	_server;

		void		checkExt();
		std::string	readFile(const char* path);
		void		parseServer(std::vector<Token>::iterator &it, std::vector<Token>::iterator end);
		
		/* Handle functions */
		void		handleListen(std::vector<Token>::iterator &it, ConfigServer &server);
		void		handleName(std::vector<Token>::iterator &it, ConfigServer &server);
		void		handleRoot(std::vector<Token>::iterator &it, ConfigServer &server);
		void		handleIndex(std::vector<Token>::iterator &it, ConfigServer &server);
		void		handleErrorPage(std::vector<Token>::iterator &it, ConfigServer &server);
		void		handleLocation(std::vector<Token>::iterator &it, ConfigServer &server);
		void		handleClientMax(std::vector<Token>::iterator &it, ConfigServer &server);
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