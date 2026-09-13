#include <sstream>
#include <cctype>
#include "http/HttpResponse.hpp"

using std::map;
using std::string;

// Class

HttpResponse::HttpResponse() : _version("HTTP/1.1"), _status(OK), _file_size(0) {}

HttpResponse::HttpResponse(const HttpResponse& src) : 
	_version(src._version),
	_status(src._status),
	_headers(src._headers),
	_body(src._body),
	_file_path(src._file_path),
	_file_size(src._file_size)
{}

HttpResponse& HttpResponse::operator=(const HttpResponse& src)
{
	if (this != &src)
	{
		_version = src._version;
		_status = src._status;
		_headers = src._headers;
		_body = src._body;
		_file_path = src._file_path;
		_file_size = src._file_size;
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
void	HttpResponse::setBody(const std::vector<char>& body) { _file_path.clear(); _file_size = 0; _body = body; }

void	HttpResponse::addHeader(const string& key, const string& value) { _headers[key] = value; }

// Other methods

string HttpResponse::serialize() const
{
	std::ostringstream out;
	map<string, string>::const_iterator it;

	out << _version << " " << _status << " " << getReasonPhrase(_status) << "\r\n";

	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		string key = it->first;
		for (size_t i = 0; i < key.size(); ++i) key[i] = std::tolower(static_cast<unsigned char>(key[i]));
		if (key != "content-length" && key != "connection" && key != "transfer-encoding")
			out << it->first << ": " << it->second << "\r\n";
	}

	out << "Content-Length: " << (_file_path.empty() ? static_cast<std::streamoff>(_body.size()) : _file_size) << "\r\n";
	out << "Connection: close\r\n\r\n";
	if (!_body.empty())
		out.write(&_body[0], _body.size());

	return out.str();
}

void HttpResponse::setFile(const string& path, std::streamoff size) { _file_path = path; _file_size = size; std::vector<char>().swap(_body); }
const string& HttpResponse::getFilePath() const { return (_file_path); }
std::streamoff HttpResponse::getFileSize() const { return (_file_size); }
