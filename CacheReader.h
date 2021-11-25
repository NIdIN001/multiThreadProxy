#pragma once

#include <thread>

#include "TcpSocket.h"
#include "Cache.h"
#include "HttpProxy.h"

class HttpProxy;

class Cache;

struct messageChunk;

class CacheReader {
private:
    Cache *cache;
    TcpSocket *writeSocket;
    HttpProxy *proxy;
    char *url;

    static std::mutex queueMutex;
    static std::condition_variable queueCondVar;

    std::list<messageChunk> messageQueue;
    std::thread *runningThread;

    void run();

public:
    bool isStop = false;

    CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy);

    ~CacheReader();

    void read(char *url);

    void stopRead();

    bool isReading();

    void notify(messageChunk chunk);
};
