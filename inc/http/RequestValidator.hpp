#ifndef REQUESTVALIDATOR_HPP
#define REQUESTVALIDATOR_HPP

#include "http/HttpStatusCode.hpp"
#include "http/HttpRequest.hpp"
#include "config/AConfig.hpp"

#include <vector>
#include <string>
#include <sstream>

class HttpRequest;

class RequestValidator
{
	private:
		HttpStatusCode	isValidRequestLine(const HttpRequest& request);
		HttpStatusCode	isValidheaders(const HttpRequest& request, const AConfig& config);
		HttpStatusCode	isValidBody(const HttpRequest& request, const AConfig& config);

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