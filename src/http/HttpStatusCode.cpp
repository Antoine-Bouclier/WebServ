#include "http/HttpStatusCode.hpp"

const char* getReasonPhrase(HttpStatusCode status)
{
	switch (status)
	{
		case NOT_MODIFIED: return "Not Modified";
		case BAD_GATEWAY: return "Bad Gateway";
		case GATEWAY_TIMEOUT: return "Gateway Timeout";
		case FOUND: return "Found";
		case TEMPORARY_REDIRECT: return "Temporary Redirect";
		case PERMANENT_REDIRECT: return "Permanent Redirect";
		case MOVED_PERMANENTLY: return "Moved Permanently";
		case OK: return "OK";
		case CREATED: return "Created";
		case CONFLICT: return "Conflict";
		case NO_CONTENT: return "No Content";
		case NOT_FOUND: return "Not Found";
		case FORBIDDEN: return "Forbidden";
		case BAD_REQUEST: return "Bad Request";
		case URI_TOO_LONG: return "URI Too Long";
		case METHOD_NOT_ALLOWED: return "Method Not Allowed";
		case LENGTH_REQUIRED: return "Length Required";
		case NOT_IMPLEMENTED: return "Not Implemented";
		case PAYLOAD_TOO_LARGE: return "Payload Too Large";
		case UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
		case VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";

		case INTERNAL_SERVER_ERROR: return "Internal Server Error";
		default: return "Unknown Status";
	}
}