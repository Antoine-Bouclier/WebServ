#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>		// std::istringstream()
#include <iostream>		// std::hex()

enum HttpParseState
{
	STATE_REQUEST_LINE,
	STATE_HEADERS,
	STATE_BODY,
	STATE_READY,
	STATE_ERROR
};

class HttpRequest
{
	private:
		/* -- Request Line -- */
		std::string	_method;
		std::string	_uri;
		std::string	_version;

		/* -- Headers -- */
		std::map<std::string, std::string>	_headers;

		/* -- Body -- */
		std::vector<char>	_body;

		/* -- Stream Control & Internal State -- */
		HttpParseState		_state;
		std::vector<char>	_buffer;
		size_t				_position_ptr;
		size_t				_content_length;
		size_t				_current_chunk_size;
		bool				_is_chunked;
		bool				_reading_chunk_headers;

		/* -- Private Parsing Sub-routine -- */
		void	parseRequestLine();
		void	parseHeaders();
		void	parseBody();

		/* -- Utils Function -- */
		void	isValidRequestLine();
		bool	searchEOL(std::vector<char>::iterator& it);
		bool	skipEOL();
		void	parseBodyContentLength();
		void	parseBodyTransferEncoding();

	public:
		HttpRequest();
		HttpRequest(const HttpRequest& src);
		HttpRequest& operator=(const HttpRequest& src);
		~HttpRequest();

		/* -- Main Function -- */
		void	feed(const char* raw_bytes, size_t bytes_count);

		/* -- Getters -- */
		const HttpParseState&	getState() const;
		const std::string&	getMethod() const;
		const std::string&	getUri() const;
		const std::string&	getVersion() const;
};

#endif