*This project has been created as part of the 42 curriculum by Arocca, Abouclie.*

# Webserv — HTTP/1.1 Server in C++98

---

## Description

**Webserv** is a lightweight, asynchronous, non-blocking HTTP/1.1 server written in C++98. The primary goal of this project is to deeply understand the hypertext transfer protocol (HTTP), network programming via POSIX sockets, I/O multiplexing, and how modern web servers (such as NGINX) process client connections and static/dynamic content under the hood.

### Key Features & Technical Overview
* **Event-Driven Architecture**: Built around a single `poll()` loop that monitors all listening sockets and active client connections simultaneously without blocking (`O_NONBLOCK` via `fcntl`).
* **HTTP/1.1 Compliance**:
  * Stateful request parsing (Request-Line, Headers, and Body)[cite: 1].
  * Support for both fixed-size (`Content-Length`) and streamed (`Transfer-Encoding: chunked`) payloads[cite: 1].
  * URI percent-decoding (`%XX`) and built-in protection against directory traversal attacks (`..`)[cite: 1].
* **Supported HTTP Methods**: `GET`, `POST`, and `DELETE`[cite: 1].
* **Static File Serving & Autoindex**:
  * Serves static files with automatic MIME-type detection (`.html`, `.css`, `.js`, `.png`, `.pdf`, `.json`, etc.)[cite: 1].
  * Dynamic directory listing generation (`autoindex on/off`)[cite: 1].
* **Redirections & Error Handling**:
  * Configurable HTTP redirects (`301`, `302`, `307`, `308`)[cite: 1].
  * Custom error pages per HTTP status code, with automatic fallback to default generated HTML pages[cite: 1].
* **CGI (Common Gateway Interface)**: Executes external scripts (e.g., PHP, Python) based on file extensions, passing request context via environment meta-variables and handling I/O through IPC (`fork`, `pipe`, `dup2`, `execve`).
* **Timeout Management**: Automatically drops idle connections or stalled requests (`CLIENT_IDLE_TIMEOUT` and `CLIENT_REQUEST_TIMEOUT`) to prevent resource exhaustion[cite: 1].

---

## Project Architecture

| Module | Key Classes / Files | Responsibility |
| :--- | :--- | :--- |
| **Parser & Config** | `Lexer`, `ConfigParser`, `AConfig`, `ConfigServer`, `ConfigLocation` | Lexical and syntactic analysis of `.conf` files, directive validation, and inheritance resolution between `server` and `location` blocks[cite: 1]. |
| **Server & Network** | `Server`, `Listener`, `Client`, `Socket` | Socket initialization, `poll()` event loop, client lifecycle management, and non-blocking read/write buffering[cite: 1]. |
| **HTTP & Routing** | `HttpRequest`, `HttpResponse`, `RequestValidator`, `Router` | Stateful HTTP parsing, protocol validation, longest-prefix route matching, and response serialization[cite: 1]. |
| **Handlers** | `RequestHandler` | Execution of HTTP methods (`GET`, `POST`, `DELETE`), autoindex HTML generation, and CGI process orchestration[cite: 1]. |

---

## Instructions

### Prerequisites
* A C++ compiler supporting the **C++98** standard (`c++` / `g++` / `clang++`).
* POSIX-compliant operating system (Linux / macOS).
* Optional: `php-cgi` or `python3` installed on the host machine to test CGI execution.

### Compilation
Clone the repository and run `make` at the root of the project to compile the binary:

```bash
git clone <your-repo-url> webserv
cd webserv
make
=======

---

## Instructions

### Prerequisites
* A C++ compiler supporting the **C++98** standard (`c++` / `g++` / `clang++`).
* POSIX-compliant operating system (Linux / macOS).
* Optional: `php-cgi` or `python3` installed on the host machine to test CGI execution.

### Compilation
Clone the repository and run `make` at the root of the project to compile the binary:

```bash
git clone <your-repo-url> webserv
cd webserv
make
```

Standard Makefile rules are available:
* `make` or `make all`: Compiles the `webserv` executable.
* `make clean`: Removes object files.
* `make fclean`: Removes object files and the executable.
* `make re`: Recompiles the project from scratch.

### Execution
Run the server by passing a valid `.conf` configuration file as an argument[cite: 1]:

```bash
./webserv config/default.conf
```

---

## Configuration File (`.conf`)

The configuration syntax is inspired by NGINX. `location` blocks automatically inherit `root`, `index`, `client_max_body_size`, and `error_page` directives from their parent `server` block if not explicitly overridden[cite: 1].

### Example Configuration:

```nginx
server {
    listen 127.0.0.1:8080;
    server_name localhost example.com;

    root ./www;
    index index.html index.htm;
    client_max_body_size 10M;

    error_page 404 /errors/404.html;
    error_page 500 502 503 504 /errors/50x.html;

    location / {
        allowed_methods GET;
        autoindex off;
    }

    location /assets {
        allowed_methods GET;
        autoindex on;
    }

    location /upload {
        allowed_methods POST DELETE;
        upload_path ./www/uploads;
        client_max_body_size 50M;
    }

    location /old-page {
        return 301 /index.html;
    }

    location /cgi-bin {
        allowed_methods GET POST;
        cgi .php /usr/bin/php-cgi;
        cgi .py /usr/bin/python3;
    }
}
```

### Quick Testing (with cURL)

* **GET Request:**
  ```bash
  curl -i http://127.0.0.1:8080/index.html
  ```
* **Chunked POST Request:**
  ```bash
  curl -i -X POST -H "Transfer-Encoding: chunked" -d "Hello World" http://127.0.0.1:8080/upload/file.txt
  ```
* **DELETE Request:**
  ```bash
  curl -i -X DELETE http://127.0.0.1:8080/upload/file.txt
  ```
* **CGI Execution:**
  ```bash
  curl -i "http://127.0.0.1:8080/cgi-bin/script.php?name=42student"
  ```

---

## Resources

### Classic References & Documentation
* **RFC 9110 (HTTP Semantics)** & **RFC 9112 (HTTP/1.1)**: Official specifications for request/response formatting, status codes, headers, and chunked transfer encoding.
* **RFC 3875 (The Common Gateway Interface - CGI/1.1)**: Specification for environment meta-variables (`REQUEST_METHOD`, `SCRIPT_FILENAME`, `QUERY_STRING`, etc.) and CGI script output parsing.
* **Beej's Guide to Network Programming**: Comprehensive guide on Unix sockets (`socket`, `bind`, `listen`, `accept`), address resolution (`getaddrinfo`), and I/O multiplexing with `poll()`.
* **NGINX Documentation**: Reference for understanding block configuration hierarchy (`server` vs `location`), longest-prefix URI matching, and directive inheritance.
* **Linux Man Pages**: Specifically `poll(2)`, `fcntl(2)`, `setsockopt(2)`, `fork(2)`, `pipe(2)`, `dup2(2)`, `execve(2)`, and `waitpid(2)`.

### Use of Artificial Intelligence
Throughout the development of **Webserv**, AI tools were used as a collaborative assistant and learning aid for specific tasks:
* **Protocol & Standard Clarification**: Clarifying edge cases in RFC 9112 (such as chunked body parsing and header validation) and RFC 3875 (such as required CGI environment variables like `REDIRECT_STATUS` for `php-cgi` and parsing CGI output headers separated by `\r\n\r\n`).
* **Implementation & Debugging Support**: Reviewing C++98 file stream operations (`std::ofstream` flags for binary file uploads) and structuring step-by-step logic for the `RequestHandler` module (`handlePost` and `handleCgi`).
* **Documentation**: Assisting in structuring, formatting, and translating this `README.md` file to comply with the 42 curriculum requirements.
