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
		void	setRoot(const std::string& root);
		void	setClientMaxBody(size_t size);
		void	addErrorPage(const int code, const std::string& path);
		void	addIndex(const std::string& one_index);

		/* -- Getters -- */
		std::string					getRoot() const;
		std::vector<std::string>	getIndex() const;
		size_t						getClientMaxBody() const;
		std::map<int, std::string>	getErrorPage()const ;
};


#endif