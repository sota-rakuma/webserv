#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

// getaddrinfoの第二引数には本来サービスで使われるポートorサービス名を指定 cf: /etc/services
static void getip(struct addrinfo **res)
{
	const struct addrinfo hints = {
		.ai_flags = AI_NUMERICSERV,
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = 0,
		.ai_addrlen = 0,
		.ai_addr = NULL,
		.ai_canonname = NULL,
		.ai_next = NULL};

	int	ret = getaddrinfo(NULL, "4242", &hints, res);
	if (ret)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(ret) << std::endl;
		std::exit(1);
	}
}

static int getsocket()
{
	int	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		perror("socket");
		std::exit(1);
	}
	return fd;
}

static int send_data(int sockfd, std::string &buff)
{
	if (write(sockfd, buff.c_str(), buff.size()) == -1) {
		perror("write");
		return 1;
	}
	return 0;
}

static int get_data(int sockfd, std::ostream &os)
{
	char buff[1024];

	if (read(sockfd, buff, 1024) < 0) {
		perror("read");
		return 1;
	}
	std::cout << "from server: " << buff << std::endl;
	return 0;
}

static void communicate_with_server(int sockfd, std::istream &is, std::ostream &os)
{
	std::string buff;

	while (std::getline(is, buff)) {
		if (send_data(sockfd, buff)) {
			break;
		}
		std::cout << "in client: sent data to server" << std::endl;
		if (get_data(sockfd, os)) {
			break;
		}
		std::cout << "get data from server" << std::endl;
	}
	if (!is.eof()) {
		std::cerr << "cannot send message" << std::endl;
	}
}

int main(int argc, char *argv[])
{
	int sockfd = getsocket();
	struct addrinfo *ret;

	getip(&ret);
	if (connect(sockfd, ret->ai_addr, sizeof(*(ret->ai_addr)))) {
		perror("connect");
		close(sockfd);
		std::exit(1);
	}
	std::cout << "connected!!!" << std::endl;
	communicate_with_server(sockfd, std::cin, std::cout);
	close(sockfd);
	std::cout << "connection end" << std::endl;
	return 0;
}
