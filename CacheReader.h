#pragma once

#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

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

    bool queueHasData = false;

    std::list<messageChunk> messageQueue;
    std::thread *runningThread;

    [[noreturn]] void run();

public:
    CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy);

    ~CacheReader();

    void read(char *url);

    void stopRead();

    bool isReading();

    void notify(messageChunk chunk);
};
