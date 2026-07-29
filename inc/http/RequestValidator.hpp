#ifndef REQUESTVALIDATOR_HPP
#define REQUESTVALIDATOR_HPP

#include "http/HttpRequest.hpp"
#include "config/AConfig.hpp"

#include <vector>
#include <string>
#include <sstream>

enum HttpStatusCode
{
	OK = 200,
	BAD_REQUEST = 400,
	LENGTH_REQUIRED = 411,
	PAYLOAD_TOO_LARGE = 413,
	URI_TOO_LONG  = 414,
	NOT_IMPLEMENTED = 501,
	VERSION_NOT_SUPPORTED = 505
};

class RequestValidator
{
	private:
		HttpStatusCode	isValidRequestLine(const HttpRequest& request);
		HttpStatusCode	isValidheaders(const HttpRequest& request, const AConfig& config);
		HttpStatusCode	isValidBody(const HttpRequest& request);

		/* -- Utils Headers Methods -- */
		HttpStatusCode	checkHost(const std::map<std::string, std::string>&	headers);
		HttpStatusCode	checkContentLength(const std::string& length_str, size_t max_body_size);

	public:
		RequestValidator();
		~RequestValidator();

		/* -- Main Method -- */
		HttpStatusCode	validate(const HttpRequest& request, const AConfig& config);
};

#endif