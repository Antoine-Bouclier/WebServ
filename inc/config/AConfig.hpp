#ifndef ACONFIG_HPP
#define ACONFIG_HPP

#include "WebServ.hpp"

class AConfig
{
protected:
	std::string					_root;
	std::vector<std::string>	_index;
	std::map<int, std::string>	_error_pages;
	size_t						_client_max_body_size;

public:
	AConfig();
	virtual ~AConfig() = 0;

	/* -- Setters -- */
	void	setClientMaxBody(size_t size);
	void	setRoot(const std::string& root);
	void	addIndex(const std::string& index);
	void	addErrorPage(const int code, const std::string& page);

	/* -- Getters -- */
	const std::string&					getRoot(void) const;
	const std::vector<std::string>&		getIndex(void) const;
	const std::map<int, std::string>&	getErrorPage(void) const;
	size_t								getClientMaxBody(void) const;
};

#endif
