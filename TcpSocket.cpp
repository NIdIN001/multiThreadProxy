#include "TcpSocket.h"
#include <iostream>


TcpSocket::TcpSocket() {
    fd = socket(AF_INET, SOCK_STREAM, 0);
}

TcpSocket::TcpSocket(int fd) {
    this->fd = fd;
}

TcpSocket::~TcpSocket() = default;

char *TcpSocket::getHostName() const {
    return hostName;
}

void TcpSocket::resolveHostName(char *hostName, addrinfo **res) {
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostName, "80", &hints, res) != 0) {
        std::cout << "Resolve for " << hostName << " failed" << std::endl;
    }
}

void TcpSocket::_connect(char *hostName, int port) {
    this->hostName = hostName;
    addrinfo *hostAddrinfo;
    resolveHostName(hostName, &hostAddrinfo);

    if (connect(fd, hostAddrinfo->ai_addr, hostAddrinfo->ai_addrlen) != 0) {
        std::cout << "Failed to connect to " << hostName << ":" << port << std::endl;
    }
}

int TcpSocket::_read(char *buf, int length) const {
    return read(fd, buf, length);
}

int TcpSocket::_write(char *buf, int length) const {
    return write(fd, buf, length);
}

void TcpSocket::_close() const {
    close(fd);
}
