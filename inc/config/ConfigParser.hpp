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
		
		/* -- Handlers location -- */
		void	handleAutoindex(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config);
		void	handlePath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config);
		void	handleUploadPath(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config);
		void	handleMethods(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config);
		void	handleCgi(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config);

		/* -- Handlers server -- */
		void	handleListen(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config);
		void	handleServerNames(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config);
		void	handleLocation(std::vector<Token>::iterator &it, std::vector<Token>::iterator end, AConfig &config);
	public:
		/* Main function */
		void	parseConfig(const char* path);
};

#endif