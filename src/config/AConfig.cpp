#include "config/AConfig.hpp"

/* -- Setters -- */
void	AConfig::setRoot(const std::string& root) { _root = root; }
void	AConfig::setClientMaxBody(size_t size) { _client_max_body_size = size; }
void	AConfig::addErrorPage(const int code, const std::string& path) { _error_pages[code] = path; }
void	AConfig::addIndex(const std::string& one_index) { _index.push_back(one_index); }


/* -- Getters -- */
const std::string&					AConfig::getRoot() const { return (_root); }
const std::vector<std::string>&		AConfig::getIndex() const { return (_index); }
const std::map<int, std::string>&	AConfig::getErrorPage()const { return (_error_pages); }
size_t								AConfig::getClientMaxBody() const { return (_client_max_body_size); }