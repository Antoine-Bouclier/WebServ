#include "config/ConfigLocation.hpp"

ConfigLocation::ConfigLocation() : _autoindex(false)
{

}

ConfigLocation::~ConfigLocation()
{

}

/* -- Setters -- */
void	ConfigLocation::setAutoindex(bool autoindex) { _autoindex = autoindex; }
void	ConfigLocation::setPath(const std::string& path) { _path = path; }
void	ConfigLocation::setUploadPath(const std::string& path) { _upload_path = path; }
void	ConfigLocation::addMethod(const std::string& method) { _methods.push_back(method); }
void	ConfigLocation::addCgi(const std::string& extension, const std::string& binaryPath) { _cgi[extension] = binaryPath; }

/* -- Getters -- */
bool										ConfigLocation::getAutoindex() const { return (_autoindex); }
const std::string&							ConfigLocation::getPath() const { return (_path); }
const std::string&							ConfigLocation::getUploadPath() const { return (_upload_path); }
const std::vector<std::string>&				ConfigLocation::getMethods() const { return (_methods); }
const std::map<std::string, std::string>&	ConfigLocation::getCgi() const { return (_cgi); }