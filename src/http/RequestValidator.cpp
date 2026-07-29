#include "http/RequestValidator.hpp"

RequestValidator::RequestValidator()
{
}

RequestValidator::~RequestValidator()
{
}

/* -- Main Method -- */
HttpStatusCode	RequestValidator::validate(const HttpRequest& request, const AConfig &config)
{
	HttpStatusCode	status;

	if (request.getState() == STATE_ERROR)
		return (BAD_REQUEST);

	status = isValidRequestLine(request);
	if (status != OK)
		return (status);
	
	status = isValidheaders(request, config);
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

HttpStatusCode	RequestValidator::isValidheaders(const HttpRequest& request, const AConfig& config)
{
	HttpStatusCode	status;
	const std::map<std::string, std::string>&			headers = request.getheaders();
	std::map<std::string, std::string>::const_iterator	content_length;
	std::map<std::string, std::string>::const_iterator	transfer_encoding;
	
	status = checkHost(headers);
	if (status != OK)
		return (status);

	content_length = headers.find("content-length");
	transfer_encoding = headers.find("transfer-encoding");
	if (content_length != headers.end() && transfer_encoding != headers.end())
		return (BAD_REQUEST);
	else if (content_length != headers.end())
	{
		status = checkContentLength(content_length->second, config.getClientMaxBody());
		if (status != OK)
			return (status);
	}
	if (transfer_encoding != headers.end())
	{
		if (transfer_encoding->second != "chunked")
			return (NOT_IMPLEMENTED);
	}
	if (request.getMethod() == "POST")
	{
		if (content_length == headers.end() && transfer_encoding == headers.end())
			return (LENGTH_REQUIRED);
	}
	return (OK);
}

HttpStatusCode	RequestValidator::isValidBody(const HttpRequest& request)
{

}

/* -- Utils Header Methods -- */
HttpStatusCode	RequestValidator::checkHost(const std::map<std::string, std::string>& headers)
{
	std::map<std::string, std::string>::const_iterator	it = headers.find("host");

	if (it == headers.end())
		return (BAD_REQUEST);
	else if (it->second.empty())
		return (BAD_REQUEST);
	return (OK);
}

HttpStatusCode	RequestValidator::checkContentLength(const std::string& length_str, size_t max_body_size)
{
	if (length_str.empty())
		return (BAD_REQUEST);

	for (size_t i = 0; i < length_str.size(); i++)
	{
		if (!isdigit(length_str[i]))
			return (BAD_REQUEST);
	}

	size_t	content_length;
	std::istringstream	iss(length_str);
	iss >> content_length;
	if (iss.fail())
		return (PAYLOAD_TOO_LARGE);
	else if (content_length > max_body_size)
		return (PAYLOAD_TOO_LARGE);
	return (OK);
}