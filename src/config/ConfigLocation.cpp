#include "config/ConfigLocation.hpp"

using std::map;
using std::vector;
using std::string;

/* -------------------- */
/* -- CANONICAL FORM -- */
/* -------------------- */

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

ConfigLocation::ConfigLocation(const ConfigLocation& src)
{
	*this = src;
}
ConfigLocation&	ConfigLocation::operator=(const ConfigLocation& src)
{
	if (this != &src)
	{
		_autoindex = src._autoindex;
		_path = src._path;
		_upload_path = src._upload_path;
		_methods = src._methods;
		_cgi = src._cgi;

		_methods_assigned = src._methods_assigned;
		_autoindex_assigned = src._autoindex_assigned;
	}
	return (*this);
}

/* ------------- */
/* -- SETTERS -- */
/* ------------- */

void	ConfigLocation::setPath(const string& path) { _path = path; }
void	ConfigLocation::setAutoindex(bool active) { _autoindex = active; }
void	ConfigLocation::setUploadPath(const string& path) { _upload_path = path; }
void	ConfigLocation::addMethod(const string& method) { _methods.push_back(method); }
void	ConfigLocation::addCgi(const string& extension, const string& binaryPath) { _cgi[extension] = binaryPath; }

void	ConfigLocation::setMethodsAssigned() { _methods_assigned = true; }
void	ConfigLocation::setAutoIndexAssigned() { _autoindex_assigned = true; }

/* ------------- */
/* -- GETTERS -- */
/* ------------- */

bool							ConfigLocation::getAutoindex() const { return (_autoindex); }
const string&					ConfigLocation::getPath() const { return (_path); }
const string&					ConfigLocation::getUploadPath() const { return (_upload_path); }
const vector<string>&			ConfigLocation::getMethods() const { return (_methods); }
const map<string, string>&		ConfigLocation::getCgi() const { return (_cgi); }

bool							ConfigLocation::isMethodsAssigned() const { return (_methods_assigned); }
bool							ConfigLocation::isAutoIndexAssigned() const { return (_autoindex_assigned); }

void	ConfigLocation::clearMethods() { _methods.clear(); }
