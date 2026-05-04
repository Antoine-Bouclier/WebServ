#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>
#include "Lexer.hpp"
#include "WebServ.hpp"
#include "ConfigServer.hpp"

typedef typename std::vector<Token>::iterator iter;

class ConfigParser
{
	public:
		ConfigParser();
		~ConfigParser();

		std::vector<ConfigServer>&			getServer(void);
		const std::vector<ConfigServer>&	getServer(void) const;
		
		/* -- Main function -- */
		void	parseConfig(const char* path);

	private:
		typedef void (ConfigParser::*Handler)(iter&, iter, AConfig&);

		std::string						_path;
		Lexer							_lexer;
		std::vector<ConfigServer>		_server;
		std::map<std::string, Handler>	_handlers;

		std::string		readFile(const char* path);
		void			parseBlock(iter &it, iter end, AConfig &config);

		/* -- Handlers AConfig -- */
		void		handleRoot(iter &it, iter end, AConfig &config);
		void		handleIndex(iter &it, iter end, AConfig &config);
		void		handleErrorPage(iter &it, iter end, AConfig &config);
		void		handleClientMax(iter &it, iter end, AConfig &config);
		
		/* -- Handlers location -- */
		void		handleCgi(iter &it, iter end, AConfig &config);
		void		handleMethods(iter &it, iter end, AConfig &config);
		void		handleAutoindex(iter &it, iter end, AConfig &config);
		void		handleUploadPath(iter &it, iter end, AConfig &config);

		/* -- Handlers server -- */
		void		handleListen(iter &it, iter end, AConfig &config);
		void		handleLocation(iter &it, iter end, AConfig &config);
		void		handleServerNames(iter &it, iter end, AConfig &config);

		template <typename T>
		struct ConfigName;

		template <typename T>
		static T& require(iter &i, AConfig& config, const std::string& directive)
		{
			T* ptr = dynamic_cast<T*>(&config);
			if (!ptr)
				throw ErrorException(directive + " only allowed in " + ConfigName<T>::name + "block.", i->line);

			return (*ptr);
		}
};

template<> struct ConfigParser::ConfigName<ConfigServer> { static const char* const name; };
template<> struct ConfigParser::ConfigName<ConfigLocation> { static const char* const name; };

#endif
