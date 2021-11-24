#pragma once

#include "TcpSocket.h"
#include "Cache.h"
#include "HttpProxy.h"

class HttpProxy;

class Cache;

struct messageChunk;

class HostSocketHandler {
private:
    TcpSocket *clientSocket;
    TcpSocket *hostSocket;
    HttpProxy *proxy;
    Cache *cache;
    char *url;

    std::thread *runningThread;

    void run();

public:
    bool isStop = false;

    HostSocketHandler(TcpSocket *clientSocket, TcpSocket *hostSocket, Cache *cache, HttpProxy *proxy);

    ~HostSocketHandler();

    void finishReadingResponse();

    void waitForNextResponse(char *url);
};
