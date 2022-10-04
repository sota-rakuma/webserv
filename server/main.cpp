#include "Server.hpp"
#include <iostream>

static void usage()
{
	std::cout << "./server [path to server.conf]" << std::endl;
	std::exit(1);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage();
	}
	(void)argv[1];

	Server server;
	server.eventMonitor();
	return 0;
}