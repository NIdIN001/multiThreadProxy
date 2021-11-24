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

    if (cache->entryIsFull(url)) {
        //std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " is reading " << url
        //          << " (entry for this URL is full)" << std::endl;
    } else {
        //std::cout << "Cache reader with write socket fd = " << writeSocket->fd << " is reading " << url
        //          << " (entry for this URL isn't full)" << std::endl;
        cache->listenToUrl(url, this);
    }

    std::cout << messageQueue.size() << " = From cache" << std::endl;

    queueHasData = true;
    queueCondVar.notify_one();
}

[[noreturn]] void CacheReader::run() {
    while (true) {
        if (isStop)
            return;

        std::unique_lock<std::mutex> locker(queueMutex);

        while (!queueHasData) {
            queueCondVar.wait(locker);
        }

        std::cout << "Writing" << std::endl;
        std::cout << "messageQueue.size() = " << messageQueue.size() << std::endl;
        while (!messageQueue.empty()) {

            messageChunk chunk = messageQueue.front();
            messageQueue.pop_front();

            int length = writeSocket->_write(chunk.buf, chunk.length);
            if (length == 0) {
                proxy->closeSession(writeSocket);
            }
            chunk.buf[chunk.length] = '\0';
            std::cout << chunk.buf;
        }
        queueHasData = false;
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
    queueHasData = true;
    queueCondVar.notify_one();
}
