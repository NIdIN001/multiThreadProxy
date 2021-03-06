#include "ServerSocket.h"

ServerSocket::ServerSocket(int port) {
    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
        perror("server socket");

    int enable = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    bind(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    listen(fd, 10);
}

ServerSocket::~ServerSocket() = default;

TcpSocket ServerSocket::_accept() const {
    return TcpSocket(accept(fd, (struct sockaddr *) nullptr, nullptr));
}

void ServerSocket::_close() const {
    close(fd);
}
