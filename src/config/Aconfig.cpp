#include "AConfig.hpp"

/* -- Setters -- */
void	AConfig::setRoot(const std::string& root)
{

}

void	AConfig::setClientMaxBody(size_t size)
{

}

void	AConfig::addErrorPage(const int code, const std::string& path)
{

}

void	AConfig::addIndex(const std::string& one_index)
{

}


/* -- Getters -- */
std::string					AConfig::getRoot() const
{

}

std::vector<std::string>	AConfig::getIndex() const
{

}

size_t						AConfig::getClientMaxBody() const
{

}

std::map<int, std::string>	AConfig::getErrorPage()const
{

}
