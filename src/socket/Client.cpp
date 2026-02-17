#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _requestComplete(false)
{

}

Client::~Client()
{

}