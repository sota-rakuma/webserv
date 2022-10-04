#include "Server.hpp"
#include <iostream>

Server::Server()
:_timeout(3000),
_listensock(),
_connections()
{
	if (_listensock.listen_(100)) {
		perror("listen");
		std::exit(1);
	}
	_pollfd.push_back((pollfd){_listensock.getSockFd(), POLLIN, 0});
}

Server::Server(int timeout)
:_timeout(timeout),
_listensock(),
_connections()
{
	if (_listensock.listen_(100)) {
		perror("listen");
		std::exit(1);
	}
	_pollfd.push_back((pollfd){_listensock.getSockFd(), POLLIN, 0});
}

Server::Server(const ListenSocket& listensock)
:_listensock(listensock),
_connections()
{
	if (_listensock.listen_(100)) {
		perror("listen");
		std::exit(1);
	}
	_pollfd.push_back((pollfd){_listensock.getSockFd(), POLLIN, 0});
}

Server::Server(const ListenSocket& listensock, int timeout)
:_timeout(timeout),
_listensock(listensock),
_connections()
{
	if (_listensock.listen_(100)) {
		perror("listen");
		std::exit(1);
	}
	_pollfd.push_back((pollfd){_listensock.getSockFd(), POLLIN, 0});
}


Server::Server(const Server& another)
:_listensock(another.getListenSock()),
_connections(another.getConnections()),
_pollfd(another.getPollfd())
{
}

Server::~Server()
{
}

const ListenSocket& Server::getListenSock() const
{
	return _listensock;
}

const std::map<int, Connection*> &Server::getConnections() const
{
	return _connections;
}

const std::vector<pollfd> &Server::getPollfd() const
{
	return _pollfd;
}

// if error occur, return -1
int Server::createConnection()
{
	sockaddr_in cli_info;
	socklen_t len = sizeof(cli_info);
	pollfd newfd;

	memset(&newfd, 0, sizeof(newfd));
	newfd.fd = accept(_listensock.getSockFd(), (sockaddr *)&cli_info, &len);
	if (newfd.fd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return CONTINUE;
		}
		perror("accept");
		return ERROR;
	}
	newfd.events = POLLIN;
	_pollfd.push_back(newfd);
	Connection *client = new Connection(newfd.fd, cli_info);
	_connections.insert(std::make_pair(newfd.fd, client));
	std::cout << "Create Connection!" << std::endl;
	return TERMINATED;
}

void Server::deleteConnection(const poll_vec_it it)
{
	int key = it->fd;

	delete _connections[key];
	_connections.erase(key);
	_pollfd.erase(it);
}

void Server::handleError(const poll_vec_it it)
{
	if (it->revents & POLLERR) {
		std::cerr << "Exception happened on device or socket" << std::endl;
	} else if (it->revents & POLLHUP) {
		std::cerr << "Connection is disconnected" << std::endl;
	} else if (it->revents & POLLNVAL) {
		std::cerr << "fd is not opend" << std::endl;
	}
	deleteConnection(it);
}

int Server::handleOutput(const poll_vec_it it)
{
	int ret = _connections[it->fd]->sendResponse();
	if (ret == TERMINATED) {
		it->events &= ~POLLOUT;
	}
	deleteConnection(it);
	return ret;
}

int Server::handleInput(const poll_vec_it it)
{
	// createConnectionのエラー無視
	if (it == _pollfd.begin()) {
		return createConnection() & ~ERROR;
	}
	int ret = _connections[it->fd]->getRequest();
	if (ret == TERMINATED
		|| _connections[it->fd]->getRbuffer().find("\x04") != std::string::npos) {
		if (_connections[it->fd]->createResponse() == ERROR) {
			return ERROR;
		}
		it->events |= POLLOUT;
	}
	return ret;
}

int Server::traversePollfd(int &ready)
{
	poll_vec_it it = _pollfd.begin();

	for (; it != _pollfd.end() && ready > 0; it++)
	{
		if (it->revents == 0) {
			continue;
		}
		if (it->revents & (POLLERR | POLLHUP | POLLNVAL)) {
			handleError(it);
		}
		else if (it->revents & POLLOUT) {
			if (handleOutput(it) == ERROR) {
				handleError(it);
			}
		}
		else {
			if (handleInput(it) == ERROR) {
				handleError(it);
			}
		}
		ready--;
	}
	return 0;
}

int Server::eventMonitor()
{
	int ready;

	for (;;)
	{
		ready = poll(_pollfd.data(), _pollfd.size(), _timeout);
		if (ready < 0) {
			perror("poll");
			return ERROR;
		} else if (ready == 0) {
			std::cout << "waiting event" << std::endl;
			sleep(1);
			continue;
		}
		traversePollfd(ready);
	}
	return 0;
}
