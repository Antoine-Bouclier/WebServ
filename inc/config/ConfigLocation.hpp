#ifndef CONFIGLOCATION_HPP
#define CONFIGLOCATION_HPP

#include "WebServ.hpp"

class ConfigLocation
{
	private:
		bool						_autoindex;
		std::string					_path;
		std::string					_cgi_extension;
		std::string					_cgi_path;
		std::string					_root;
		std::string					_upload_path;
		std::vector<std::string>	_methods;
	public:
		ConfigLocation();
		~ConfigLocation();
};

#endif