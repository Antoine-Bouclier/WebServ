#include "http/HttpStatusCode.hpp"

const char* getReasonPhrase(HttpStatusCode status)
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