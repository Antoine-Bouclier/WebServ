#ifndef CONFIGLOCATION_HPP
#define CONFIGLOCATION_HPP

#include "WebServ.hpp"
#include "AConfig.hpp"

class ConfigLocation : public AConfig
{
	private:
		bool			_autoindex;
		std::string		_path;
		std::string		_upload_path;

		std::vector<std::string>	_methods;

		std::map<std::string, std::string>	_cgi;
	public:
		ConfigLocation();
		~ConfigLocation();

		/* -- Setters -- */
		void	setAutoindex(bool autoindex);
		void	setPath(const std::string& path);
		void	setUploadPath(const std::string& path);
		void	addMethod(const std::string& method);
		void	addCgi(const std::string& extension, const std::string& binaryPath);

		/* -- Getters -- */
		bool										getAutoindex() const;
		const std::string&							getPath() const;
		const std::string&							getUploadPath() const;
		const std::vector<std::string>&				getMethods() const;
		const std::map<std::string, std::string>&	getCgi() const;
};

#endif