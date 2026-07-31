#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "string"
#include "vector"
#include "map"
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
};

#endif