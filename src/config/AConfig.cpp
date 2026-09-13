#include "config/AConfig.hpp"

using std::map;
using std::string;
using std::vector;

AConfig::AConfig() : _client_max_body_size(0), _max_size_assigned(false) {}
AConfig::~AConfig() {}

/* -- Setters -- */
void	AConfig::setRoot(const string& root) { _root = root; }
void	AConfig::setIndex(const vector<string>& index) { _index = index; }
void	AConfig::addIndex(const string& index) { _index.push_back(index); }
void	AConfig::setClientMaxBody(size_t size) { _client_max_body_size = size; }
void	AConfig::addErrorPage(const int code, const string& page) { _error_pages[code] = page; }
void	AConfig::setErrorPages(const map<int, string>& error_pages) { _error_pages = error_pages; }

void	AConfig::setAssignedClientMaxBodySize() { _max_size_assigned = true; }

/* -- Getters -- */
const string&			AConfig::getRoot() const			{ return (_root); }
const vector<string>&	AConfig::getIndex() const			{ return (_index); }
const map<int, string>&	AConfig::getErrorPage() const		{ return (_error_pages); }
size_t					AConfig::getClientMaxBody() const	{ return (_client_max_body_size); }

const std::string	AConfig::getErrorPagePath(int error_code) const
{
	std::map<int, string>::const_iterator it = _error_pages.find(error_code);
	if (it != _error_pages.end())
		return (it->second);
	return ("");
}

bool	AConfig::isClientMaxBodySizeAssigned() const { return (_max_size_assigned); }

AConfig::AConfig(const AConfig& other) : _root(other._root), _index(other._index), _error_pages(other._error_pages), _client_max_body_size(other._client_max_body_size), _max_size_assigned(other._max_size_assigned) {}

AConfig& AConfig::operator=(const AConfig& other)
{
	if (this != &other)
	{
		_root = other._root;
		_index = other._index;
		_error_pages = other._error_pages;
		_client_max_body_size = other._client_max_body_size;
		_max_size_assigned = other._max_size_assigned;
	}
	return (*this);
}
