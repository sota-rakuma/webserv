#ifndef SERVER_HPP
#define SERVER_HPP

#include "ListenSocket.hpp"
#include "Connection.hpp"
#include <vector>
#include <map>
#include <poll.h>

class Server
{
private:
	typedef std::vector<pollfd>::iterator poll_vec_it;
	int _timeout;
	ListenSocket _listensock;
	std::map<int, Connection *> _connections;
	std::vector<pollfd> _pollfd;
	int traversePollfd(int&);
	void handleError(const poll_vec_it);
	int handleOutput(const poll_vec_it);
	int handleInput(const poll_vec_it);
	void deleteConnection(const poll_vec_it);
public:
	Server();
	Server(int);
	Server(const ListenSocket&);
	Server(const ListenSocket&, int);
	Server(const Server&);
	~Server();
	const ListenSocket& getListenSock() const;
	const std::map<int, Connection*> &getConnections() const;
	const std::vector<pollfd> &getPollfd() const;
	int createConnection();
	int eventMonitor();
};


#endif /* SERVER_HPP */
