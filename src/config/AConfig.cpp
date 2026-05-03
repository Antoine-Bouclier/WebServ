#include "AConfig.hpp"

using std::map;
using std::string;
using std::vector;

AConfig::AConfig() : _client_max_body_size(0), _max_size_assigned(false) {}
AConfig::~AConfig() {}

/* -- Setters -- */
void	AConfig::setRoot(const string& root) { _root = root; }
void	AConfig::addIndex(const string& index) { _index.push_back(index); }
void	AConfig::setClientMaxBody(size_t size) { _client_max_body_size = size; }
void	AConfig::addErrorPage(const int code, const string& page) { _error_pages[code] = page; }
void	AConfig::setAssignedClientMaxBodySize() { _max_size_assigned = true; }

/* -- Getters -- */
const string&					AConfig::getRoot() const { return (_root); }
const vector<string>&			AConfig::getIndex() const { return (_index); }
const map<int, string>&			AConfig::getErrorPage() const { return (_error_pages); }
size_t							AConfig::getClientMaxBody() const { return (_client_max_body_size); }
bool							AConfig::isClientMaxBodySizeAssigned() const { return (_max_size_assigned); }
