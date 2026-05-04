#include "config/ConfigLocation.hpp"

using std::map;
using std::vector;
using std::string;

ConfigLocation::ConfigLocation()
	:	_autoindex(false),
		_methods_assigned(false),
		_autoindex_assigned(false)
{
	_methods.push_back("GET");
	_methods.push_back("POST");
	_methods.push_back("DELETE");
}

ConfigLocation::~ConfigLocation() {}

void	ConfigLocation::clearMethods() { _methods.clear(); }

/* -- Setters -- */
void	ConfigLocation::setPath(const string& path) { _path = path; }
void	ConfigLocation::setAutoindex(bool active) { _autoindex = active; }
void	ConfigLocation::setUploadPath(const string& path) { _upload_path = path; }
void	ConfigLocation::addMethod(const string& method) { _methods.push_back(method); }
void	ConfigLocation::addCgi(const string& extension, const string& binaryPath) { _cgi[extension] = binaryPath; }

void	ConfigLocation::setMethodsAssigned() { _methods_assigned = true; }
void	ConfigLocation::setAutoIndexAssigned() { _autoindex_assigned = true; }

/* -- Getters -- */
bool							ConfigLocation::getAutoindex() const { return (_autoindex); }
const string&					ConfigLocation::getPath() const { return (_path); }
const string&					ConfigLocation::getUploadPath() const { return (_upload_path); }
const vector<string>&			ConfigLocation::getMethods() const { return (_methods); }
const map<string, string>&		ConfigLocation::getCgi() const { return (_cgi); }

bool							ConfigLocation::isMethodsAssigned() const { return (_methods_assigned); }
bool							ConfigLocation::isAutoIndexAssigned() const { return (_autoindex_assigned); }
