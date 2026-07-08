#ifndef REQUESTVALIDATOR_HPP
#define REQUESTVALIDATOR_HPP

#include "http/HttpRequest.hpp"

#include <vector>
#include <string>

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
		HttpStatusCode	isValidheaders(const HttpRequest& request);
		HttpStatusCode	isValidBody(const HttpRequest& request);

	public:
		RequestValidator();
		~RequestValidator();

		/* -- Main Method -- */
		HttpStatusCode	validate(const HttpRequest& request);
};

#endif