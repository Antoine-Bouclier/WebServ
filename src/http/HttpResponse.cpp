#include "http/HttpResponse.hpp"

HttpResponse::HttpResponse() : _version("HTTP/1.1")
{

}

HttpResponse::HttpResponse(const HttpResponse& src) : 
	_version(src._version),
	_status(src._status),
	_headers(src._headers),
	_body(src._body)
{}

HttpResponse& HttpResponse::operator=(const HttpResponse& src)
{
	if (this != &src)
	{
		_version = src._version;
		_status = src._status;
		_headers = src._headers;
		_body = src._body;
	}
	return (*this);
}

HttpResponse::~HttpResponse()
{

}

/* -- Getters -- */
const std::string&	HttpResponse::getVersion() const { return (_version); }
const HttpStatusCode&	HttpResponse::getStatus() const { return (_status); }
const std::map<std::string, std::string>&	HttpResponse::getHeaders() const { return (_headers); }
const std::vector<char>&	HttpResponse::getBody() const {return (_body); }

/* -- Setters -- */
void	HttpResponse::setVersion(const std::string& version) { _version = version; }
void	HttpResponse::setStatus(const HttpStatusCode& status) { _status = status; }
void	HttpResponse::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
void	HttpResponse::setBody(const std::vector<char>& body) { _body = body; }

void	HttpResponse::addHeader(const std::string& key, const std::string& value) { _headers[key] = value; }