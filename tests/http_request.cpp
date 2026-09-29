#include "http/HttpRequest.hpp"
#include "config/ConfigServer.hpp"
#include <cassert>
#include <iostream>

using std::string;

static void feed(HttpRequest& request, const string& data, const ConfigServer& config)
{
	request.feed(data.data(), data.size());
	if (request.getState() == STATE_HEADERS_DONE) request.startBody(config);
}

static void checkResult(const HttpRequest& request, const string& body, const string& path, const string& query)
{
	assert(request.getState() == STATE_READY);
	assert(string(request.getBody().begin(), request.getBody().end()) == body);
	assert(request.getPath() == path);
	assert(request.getQuery() == query);
}

static void checkSplits(const string& wire, const string& body, const string& path, const string& query)
{
	ConfigServer config;
	config.setClientMaxBody(1024);
	for (size_t cut = 0; cut <= wire.size(); ++cut)
	{
		HttpRequest request;
		feed(request, wire.substr(0, cut), config);
		HttpRequest copy(request);
		HttpRequest assigned;
		assigned = request;
		feed(request, wire.substr(cut), config);
		feed(copy, wire.substr(cut), config);
		feed(assigned, wire.substr(cut), config);
		checkResult(request, body, path, query);
		checkResult(copy, body, path, query);
		checkResult(assigned, body, path, query);
	}
	HttpRequest request;
	for (size_t i = 0; i < wire.size(); ++i) feed(request, wire.substr(i, 1), config);
	checkResult(request, body, path, query);
}

static void checkError(const string& wire, HttpStatusCode status)
{
	ConfigServer config;
	config.setClientMaxBody(8);
	for (size_t cut = 0; cut <= wire.size(); ++cut)
	{
		HttpRequest request;
		feed(request, wire.substr(0, cut), config);
		feed(request, wire.substr(cut), config);
		assert(request.getState() == STATE_ERROR);
		assert(request.getStatusCode() == status);
	}
}

int main()
{
	string post = "POST /upload HTTP/1.1\r\nHost: localhost\r\n";
	string chunked = post + "Transfer-Encoding: chunked\r\n\r\n";
	string binary("a\0b\xff", 4);
	checkSplits("GET /a%20b?q=%2F#fragment HTTP/1.1\r\nHost: localhost\r\n\r\n", "", "/a b", "q=%2F");
	checkSplits("GET /%252e%252e HTTP/1.1\r\nHost: localhost\r\n\r\n", "", "/%2e%2e", "");
	checkSplits("GET /./cgi-bin//echo.py?q=%2F HTTP/1.1\r\nHost: localhost\r\n\r\n", "", "/cgi-bin/echo.py", "q=%2F");
	checkSplits("GET /%2e%2Fcgi-bin/echo.py HTTP/1.1\r\nHost: localhost\r\n\r\n", "", "/cgi-bin/echo.py", "");
	checkSplits("GET /a/b/. HTTP/1.1\r\nHost: localhost\r\n\r\n", "", "/a/b/", "");
	checkSplits("GET //a///b// HTTP/1.1\r\nHost: localhost\r\n\r\n", "", "/a/b/", "");
	checkSplits("GET /./ HTTP/1.1\r\nHost: localhost\r\n\r\n", "", "/", "");
	checkSplits(post + "Content-Length: 4\r\n\r\n" + binary, binary, "/upload", "");
	checkSplits(post + "Content-Length: 0\r\n\r\n", "", "/upload", "");
	checkSplits(chunked + "1\r\na\r\n3\r\n" + binary.substr(1) + "\r\n0\r\n\r\n", binary, "/upload", "");
	checkSplits(chunked + "A\r\n0123456789\r\n0\r\n\r\n", "0123456789", "/upload", "");
	checkSplits(chunked + "0\r\n\r\n", "", "/upload", "");

	checkError("GET /% HTTP/1.1\r\nHost: localhost\r\n\r\n", BAD_REQUEST);
	checkError("GET /%GG HTTP/1.1\r\nHost: localhost\r\n\r\n", BAD_REQUEST);
	checkError("GET /%00 HTTP/1.1\r\nHost: localhost\r\n\r\n", BAD_REQUEST);
	checkError("GET /a/%2e%2e/x HTTP/1.1\r\nHost: localhost\r\n\r\n", FORBIDDEN);
	checkError("GET / HTTP/1.1\r\n\r\n", BAD_REQUEST);
	checkError(post + "Host: duplicate\r\n\r\n", BAD_REQUEST);
	checkError(post + "Bad Name: x\r\n\r\n", BAD_REQUEST);
	checkError(post + "X-Test: bad\x01\r\n\r\n", BAD_REQUEST);
	checkError(post + "Content-Length: 0\r\nContent-Length: 0\r\n\r\n", BAD_REQUEST);
	checkError(post + "Content-Length: 0\r\nTransfer-Encoding: chunked\r\n\r\n", BAD_REQUEST);
	checkError(post + "Content-Length: 9\r\n\r\n", PAYLOAD_TOO_LARGE);
	checkError(post + "Content-Length: -1\r\n\r\n", BAD_REQUEST);
	checkError(post + "Content-Length: " + string(40, '9') + "\r\n\r\n", BAD_REQUEST);
	checkError(chunked + "9\r\n", PAYLOAD_TOO_LARGE);
	checkError(chunked + "5\r\nhello\r\n4\r\n", PAYLOAD_TOO_LARGE);
	checkError(chunked + "G\r\n", BAD_REQUEST);
	checkError(chunked + string(40, 'F') + "\r\n", BAD_REQUEST);
	checkError(chunked + "1\r\naXX", BAD_REQUEST);
	checkError(chunked + "0\r\nX-Trailer: x\r\n\r\n", BAD_REQUEST);

	ConfigServer config;
	config.setClientMaxBody(8);
	HttpRequest request;
	feed(request, post + "Content-Length: 4\r\nX-Test:\t Value \t\r\n\r\nab", config);
	assert(request.getState() == STATE_BODY);
	assert(request.getheaders().find("x-test")->second == "Value");
	feed(request, "cd", config);
	checkResult(request, "abcd", "/upload", "");
	request.releaseBody();
	assert(request.getBody().empty());
	HttpRequest oversized;
	feed(oversized, "GET / HTTP/1.1\r\nHost: localhost\r\nX-Long: " + string(MAX_HEADER_SIZE, 'a'), config);
	assert(oversized.getState() == STATE_ERROR);
	std::cout << "Parser checks passed: every split, bytewise input, copies, limits, headers and URI decoding.\n";
}
