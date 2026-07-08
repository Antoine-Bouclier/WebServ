#include "http/RequestValidator.hpp"

RequestValidator::RequestValidator()
{
}

RequestValidator::~RequestValidator()
{
}

/* -- Main Method -- */
HttpStatusCode	RequestValidator::validate(const HttpRequest& request)
{
	HttpStatusCode	status;

	if (request.getState() == STATE_ERROR)
		return (BAD_REQUEST);

	status = isValidRequestLine(request);
	if (status != OK)
		return (status);
	
	status = isValidheaders(request);
	if (status != OK)
		return (status);

	return (status);
}

HttpStatusCode	RequestValidator::isValidRequestLine(const HttpRequest& request)
{
	if (request.getMethod() != "GET" && request.getMethod() != "POST" && request.getMethod() != "DELETE")
		return (NOT_IMPLEMENTED);
	if (request.getUri().empty() || request.getUri()[0] != '/')
		return (BAD_REQUEST);
	if (request.getUri().size() > 8192)
		return (URI_TOO_LONG);
	if (request.getVersion() != "HTTP/1.1")
		return (VERSION_NOT_SUPPORTED);
	return (OK);
}

HttpStatusCode	RequestValidator::isValidheaders(const HttpRequest& request)
{
	std::map<std::string, std::string>				headers = request.getheaders();
	std::map<std::string, std::string>::iterator	found;
	
	found = headers.find("host");
	if (found == headers.end())
		return (BAD_REQUEST);
	
	if (request.getMethod() == "POST")
	{
		if (headers.find("content-length") == headers.end() && headers.find("transfer-encoding") == headers.end())
			return (LENGTH_REQUIRED);
		else if (headers.find("content-length") != headers.end())
		{
			
		}
	}
}

HttpStatusCode	RequestValidator::isValidBody(const HttpRequest& request)
{

}
