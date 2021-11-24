#pragma once

#include "Cache.h"
#include "TcpSocket.h"
#include "HttpProxy.h"

class HttpProxy;

struct messageChunk;

class ClientSocketHandler {
private:
    static const int BUFFER_SIZE = 2048;

    TcpSocket *clientSocket;
    TcpSocket *hostSocket = nullptr;
    HttpProxy *proxy;

    bool parseRequest(char *request);

    void run();

    std::thread *runningThread;

public:
    bool isStop = false;

    ClientSocketHandler(TcpSocket *clientSocket, HttpProxy *proxy);

    ~ClientSocketHandler();

    void setHostSocket(TcpSocket *hostSocket);

    int getClientFd();
};
