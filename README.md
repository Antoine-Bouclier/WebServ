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
