#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <ios>
#include <map>
#include <string>
#include <vector>
#include "http/HttpStatusCode.hpp"

class HttpResponse
{
	private:
		std::string		_version;
		HttpStatusCode	_status;
		std::map<std::string, std::string>	_headers;
		std::vector<char>	_body;
		std::string _file_path;
		std::streamoff _file_size;

	public:
		HttpResponse();
		HttpResponse(const HttpResponse& src);
		HttpResponse& operator=(const HttpResponse& src);
		~HttpResponse();

		/* -- Getters & Setters -- */
		const std::string&							getVersion() const;
		const HttpStatusCode&						getStatus() const;
		const std::map<std::string, std::string>&	getHeaders() const;
		const std::vector<char>&					getBody() const;

		void setFile(const std::string& path, std::streamoff size);
		const std::string& getFilePath() const;
		std::streamoff getFileSize() const;

		void	setVersion(const std::string& version);
		void	setStatus(const HttpStatusCode& status);
		void	setHeaders(const std::map<std::string, std::string>& headers);
		void	setBody(const std::vector<char>& body);

		void	addHeader(const std::string& key, const std::string& value);

		std::string	serialize() const;

};

#endif