#include "CacheReader.h"

std::mutex CacheReader::queueMutex;
std::condition_variable CacheReader::queueCondVar;

CacheReader::CacheReader(Cache *cache, TcpSocket *writeSocket, HttpProxy *proxy) {
    this->cache = cache;
    this->writeSocket = writeSocket;
    this->proxy = proxy;
    this->url = nullptr;

    this->runningThread = new std::thread(&CacheReader::run, this);
    runningThread->detach();
}

CacheReader::~CacheReader() = default;

void CacheReader::read(char *url) {
    std::unique_lock<std::mutex> locker(queueMutex);

    this->url = url;
    messageQueue = cache->getChunks(url);

    if (!cache->entryIsFull(url)) {
        cache->listenToUrl(url, this);
    }

    std::cout << messageQueue.size() << " = From cache" << std::endl;

    queueCondVar.notify_one();
}

void CacheReader::run() {
    while (true) {
        if (isStop)
            return;

        std::unique_lock<std::mutex> locker(queueMutex);

        while (messageQueue.empty()) {
            queueCondVar.wait(locker);
        }

        while (!messageQueue.empty()) {
            messageChunk chunk = messageQueue.front();
            messageQueue.pop_front();

            int length = writeSocket->_write(chunk.buf, chunk.length);
            if (length == 0) {
                proxy->closeSession(writeSocket);
            }
        }
    }
}

void CacheReader::stopRead() {
    if (url != nullptr) {
        cache->stopListening(this);
        std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " has finished reading " << url
                  << std::endl;
        url = nullptr;
    }
}

bool CacheReader::isReading() {
    return url != nullptr;
}

void CacheReader::notify(messageChunk chunk) {
    std::unique_lock<std::mutex> locker(queueMutex);
    messageQueue.push_back(chunk);
    queueCondVar.notify_one();
}
