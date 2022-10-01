#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

static int InitSocket()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket");
		std::exit(1);
	}

	struct sockaddr_in info;
	memset(&info, 0, sizeof(info));
	info.sin_family = AF_INET;
	info.sin_port = htons(4242);
	in_addr_t ipaddr = inet_addr("127.0.0.1");
	if (ipaddr == -1) {
		std::cerr << "cannot find ip address" << std::endl;
		close(sockfd);
	}
	info.sin_addr.s_addr = ipaddr;
	if (bind(sockfd, (struct sockaddr *)&info, sizeof(info))) {
		perror("bind");
		close(sockfd);
		std::exit(1);
	}

	if (listen(sockfd, 100)) {
		perror("listen");
		close(sockfd);
		std::exit(1);
	}
	return (sockfd);
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

static int send_back(int client_sock, char *buff)
{
	if (write(client_sock, buff, strlen(buff) + 1) == -1) {
		perror("write");
		return 1;
	}
	return 0;
}

static int communicate_with_client(int client_sock)
{
	char buff[1024];
	bool is_end = false;
	size_t nb;

	while ((nb = read(client_sock, buff, 1023)) > 0 || !is_end) {
		buff[nb] = '\0';
		std::cout << "from client: " << buff << std::endl;
		if (strcmp(buff, "EOF") == 0) {
			strncpy(buff, "bye!", 5);
			is_end = true;
		}
		if (send_back(client_sock, buff)) {
			return 1;
		}
		std::cout << "sent data to client!" << std::endl;
	}
	if (nb < 0) {
		perror("read");
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int sockfd = InitSocket();
	struct sockaddr_in client;
	size_t	cli_sock_len = sizeof(client);

	while (1)
	{
		memset(&client, 0, cli_sock_len);
		int client_fd = accept(sockfd, (struct sockaddr *)&client, (socklen_t *)&cli_sock_len);
		// 2回目の接続以降のかつ待機中のクライアントがいない場合に、これがないとエラーになる？
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			usleep(500);
			std::cout << "server wait for client" << std::endl;
			continue;
		}
		if (client_fd == -1) {
			perror("accept");
			close(sockfd);
			std::exit(1);
		}
		communicate_with_client(client_fd);
		close(client_fd);
	}

	close(sockfd);
	return 0;
}
