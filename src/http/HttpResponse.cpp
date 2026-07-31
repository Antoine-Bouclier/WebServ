#include "http/HttpResponse.hpp"

HttpResponse::HttpResponse()
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
