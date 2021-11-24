#pragma once

#include <unistd.h>
#include <netdb.h>

class TcpSocket {
public:
    int fd;
    char *hostName;

    void _connect(char *hostName, int port);

    int _read(char *buf, int length) const;

    int _write(char *buf, int length) const;

    void _close() const;

    TcpSocket();

    explicit TcpSocket(int fd);

    ~TcpSocket();

    char *getHostName() const;

private:
    void resolveHostName(char *hostName, addrinfo **res);
};

