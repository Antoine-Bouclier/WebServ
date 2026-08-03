#include <sstream>
#include "http/HttpResponse.hpp"

using std::map;
using std::string;

static const char* getReasonPhrase(HttpStatusCode status);

// Class

HttpResponse::HttpResponse() : _version("HTTP/1.1"), _status(OK) {}

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
const string&				HttpResponse::getVersion() const { return (_version); }
const HttpStatusCode&		HttpResponse::getStatus() const { return (_status); }
const map<string, string>&	HttpResponse::getHeaders() const { return (_headers); }
const std::vector<char>&	HttpResponse::getBody() const {return (_body); }

/* -- Setters -- */
void	HttpResponse::setVersion(const string& version) { _version = version; }
void	HttpResponse::setStatus(const HttpStatusCode& status) { _status = status; }
void	HttpResponse::setHeaders(const map<string, string>& headers) { _headers = headers; }
void	HttpResponse::setBody(const std::vector<char>& body) { _body = body; }

void	HttpResponse::addHeader(const string& key, const string& value) { _headers[key] = value; }

// Other methods

string HttpResponse::serialize() const
{
	std::ostringstream out;
	map<string, string>::const_iterator it;

	out << _version << " " << _status << " " << getReasonPhrase(_status) << "\r\n";

	for (it = _headers.begin(); it != _headers.end(); ++it)
		out << it->first << ": " << it->second << "\r\n";

	out << "Content-Length: " << _body.size() << "\r\n";
	out << "Connection: close\r\n\r\n";
	out << string(_body.begin(), _body.end());

	return out.str();
}

// Static methods

static const char* getReasonPhrase(HttpStatusCode status)
{
	switch (status)
	{
		case OK: return "OK";
		case NOT_FOUND: return "Not Found";
		case BAD_REQUEST: return "Bad Request";
		case URI_TOO_LONG: return "URI Too Long";
		case LENGTH_REQUIRED: return "Length Required";
		case NOT_IMPLEMENTED: return "Not Implemented";
		case PAYLOAD_TOO_LARGE: return "Payload Too Large";
		case VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";

		default: return "Internal Server Error";
	}
}
