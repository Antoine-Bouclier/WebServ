#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>
#include <vector>
#include "http/HttpStatusCode.hpp"

class HttpResponse
{
	private:
		std::string		_version;
		HttpStatusCode	_status;
		std::map<std::string, std::string>	_headers;
		std::vector<char>	_body;

	public:
		HttpResponse();
		HttpResponse(const HttpResponse& src);
		HttpResponse& operator=(const HttpResponse& src);
		~HttpResponse();

		/* -- Getters & Setters -- */
		const std::string&							getVersion() const;
		const HttpStatusCode&						getStatus() const;
		const std::map<std::string, std::string>&	getHeaders() const;
		const std::vector<char>&					getBody() const;

		void	setVersion(const std::string& version);
		void	setStatus(const HttpStatusCode& status);
		void	setHeaders(const std::map<std::string, std::string>& headers);
		void	setBody(const std::vector<char>& body);

		void	addHeader(const std::string& key, const std::string& value);

		std::string	serialize() const;

};

#endif