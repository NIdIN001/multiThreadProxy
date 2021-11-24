#include "HostSocketHandler.h"

HostSocketHandler::HostSocketHandler(TcpSocket *clientSocket, TcpSocket *hostSocket, Cache *cache, HttpProxy *proxy) {
    this->clientSocket = clientSocket;
    this->hostSocket = hostSocket;
    this->proxy = proxy;
    this->cache = cache;
    this->url = nullptr;

    this->runningThread = new std::thread(&HostSocketHandler::run, this);
    runningThread->detach();
}

HostSocketHandler::~HostSocketHandler() = default;

void HostSocketHandler::run() {
    while (true) {
        if (isStop)
            return;

        char *buf = new char[MAX_CHUNK_SIZE];
        int length = hostSocket->_read(buf, MAX_CHUNK_SIZE);

        if (length <= 0) {
            proxy->closeSession(hostSocket);
            return;
        }

        messageChunk chunk;
        chunk.buf = buf;
        chunk.length = length;
        cache->addChunk(url, chunk);

        length = clientSocket->_write(buf, length);

        std::cout << "send " << length << std::endl;
        if (length <= 0) {
            proxy->closeSession(clientSocket);
            return;
        }
    }
}

void HostSocketHandler::finishReadingResponse() {
    if (url != nullptr) {
        cache->makeEntryFull(url);

        std::cout << "HostSocketHandler with hostFd = " << hostSocket->fd << " has finished reading " << url
                  << std::endl;
    }
}

void HostSocketHandler::waitForNextResponse(char *url) {
    this->url = url;

    std::cout << "HostSocketHandler with hostFd = " << hostSocket->fd << " starting to read " << url << std::endl;

    cache->addEntry(url);
}
