#ifndef CONFIGLOCATION_HPP
#define CONFIGLOCATION_HPP

#include "WebServ.hpp"
#include "AConfig.hpp"

class ConfigLocation : public AConfig
{
public:
	ConfigLocation();
	~ConfigLocation();

	void	clearMethods(void);

	/* -- Setters -- */
	void	setAutoindex(bool active);
	void	setPath(const std::string& path);
	void	addMethod(const std::string& method);
	void	setUploadPath(const std::string& path);
	void	addCgi(const std::string& extension, const std::string& binaryPath);

	void	setMethodsAssigned(void);
	void	setAutoIndexAssigned(void);

	/* -- Getters -- */
	bool										getAutoindex(void) const;
	const std::string&							getPath(void) const;
	const std::string&							getUploadPath(void) const;
	const std::vector<std::string>&				getMethods(void) const;
	const std::map<std::string, std::string>&	getCgi(void) const;

	bool										isMethodsAssigned(void) const;
	bool										isAutoIndexAssigned(void) const;

private:
	bool								_autoindex;
	std::string							_path;
	std::string							_upload_path;
	std::vector<std::string>			_methods;
	std::map<std::string, std::string>	_cgi;

	bool								_methods_assigned;
	bool								_autoindex_assigned;
};

#endif
