#pragma once

#include <sys/poll.h>
#include <iostream>

#include "TcpSocket.h"

class ServerSocket {
public:
    int fd;

    explicit ServerSocket(int port);

    ~ServerSocket();

    TcpSocket _accept() const;

    void _close() const;
};
