#include "HttpProxy.h"


std::mutex HttpProxy::proxyEntriesMutex;

HttpProxy::HttpProxy(int listenPort) : serverSocket(listenPort) {
    cache = new Cache();
}

HttpProxy::~HttpProxy() = default;

void HttpProxy::acceptClient() {
    TcpSocket *clientSocket = new TcpSocket(serverSocket._accept());

    if (clientSocket->fd == -1) {
        std::cerr << "accept error" << std::endl;
    }

    ProxyEntry proxyEntry;
    proxyEntry.clientSocket = clientSocket;
    proxyEntry.clientSocketHandler = new ClientSocketHandler(clientSocket, this);
    proxyEntry.hostSocket = nullptr;
    proxyEntry.hostSocketHandler = nullptr;
    proxyEntry.cacheReader = new CacheReader(cache, clientSocket, this);

    std::unique_lock<std::mutex> locker(proxyEntriesMutex);
    proxyEntries.push_back(proxyEntry);
}

void HttpProxy::start() {
    while (true) {
        acceptClient();
    }
}

void HttpProxy::closeSession(int proxyEntryIndex) {
    ProxyEntry proxyEntry = proxyEntries[proxyEntryIndex];

    if (proxyEntry.hostSocketHandler != nullptr)
        proxyEntry.hostSocketHandler->isStop = true;

    if (proxyEntry.clientSocketHandler != nullptr)
        proxyEntry.clientSocketHandler->isStop = true;

    if (proxyEntry.cacheReader != nullptr)
        proxyEntry.cacheReader->isStop = true;

    proxyEntry.clientSocket->_close();
    delete proxyEntry.clientSocket;
    delete proxyEntry.clientSocketHandler;

    if (proxyEntry.hostSocket != nullptr) {
        proxyEntry.hostSocket->_close();
        delete proxyEntry.hostSocket;
        proxyEntry.hostSocketHandler->finishReadingResponse();
        delete proxyEntry.hostSocketHandler;
    }

    if (proxyEntry.cacheReader->isReading()) {
        proxyEntry.cacheReader->stopRead();
    }
    delete proxyEntry.cacheReader;

    proxyEntries.erase(proxyEntries.begin() + proxyEntryIndex);
}


void HttpProxy::gotNewRequest(ClientSocketHandler *clientSocketHandler, char *url) {
    std::unique_lock<std::mutex> locker(proxyEntriesMutex);

    if (!cache->contains(url)) {
        for (ProxyEntry &proxyEntry : proxyEntries) {
            if (proxyEntry.clientSocketHandler == clientSocketHandler) {

                int hostNameLength = strstr(url, "/") - url;
                char *hostName = new char[hostNameLength + 1];
                memccpy(hostName, url, 0, hostNameLength);
                hostName[hostNameLength] = '\0';
                if (proxyEntry.hostSocket == nullptr) {
                    proxyEntry.hostSocket = new TcpSocket();
                    proxyEntry.hostSocket->_connect(hostName, 80);

                    clientSocketHandler->setHostSocket(proxyEntry.hostSocket);

                    proxyEntry.hostSocketHandler = new HostSocketHandler(proxyEntry.clientSocket, proxyEntry.hostSocket,
                                                                         cache, this);
                } else if (strcmp(hostName, proxyEntry.hostSocket->getHostName()) != 0) {
                    proxyEntry.hostSocket->_close();
                    delete proxyEntry.hostSocket;

                    proxyEntry.hostSocket = new TcpSocket();
                    proxyEntry.hostSocket->_connect(hostName, 80);
                    clientSocketHandler->setHostSocket(proxyEntry.hostSocket);
                }

                if (proxyEntry.cacheReader->isReading()) {
                    proxyEntry.cacheReader->stopRead();
                } else {
                    proxyEntry.hostSocketHandler->finishReadingResponse();
                    proxyEntry.hostSocketHandler->waitForNextResponse(url);
                }
                break;
            }
        }
    } else {
        std::cout << "Data for " << url << " is in cache" << std::endl;
        for (ProxyEntry &proxyEntry : proxyEntries) {
            if (proxyEntry.clientSocketHandler == clientSocketHandler) {
                proxyEntry.cacheReader->read(url);
            }
        }
    }
}

void HttpProxy::closeSession(TcpSocket *socket) {
    std::unique_lock<std::mutex> locker(proxyEntriesMutex);
    for (int i = 0; i < proxyEntries.size(); i++) {
        if (proxyEntries[i].clientSocket == socket || proxyEntries[i].hostSocket == socket) {
            closeSession(i);
            return;
        }
    }
}

void HttpProxy::clear() {
    std::unique_lock<std::mutex> locker(proxyEntriesMutex);
    for (ProxyEntry proxyEntry : proxyEntries) {
        delete proxyEntry.cacheReader;

        proxyEntry.clientSocket->_close();
        delete proxyEntry.clientSocket;
        delete proxyEntry.clientSocketHandler;

        if (proxyEntry.hostSocket != nullptr) {
            proxyEntry.hostSocket->_close();
            delete proxyEntry.hostSocket;
            delete proxyEntry.hostSocketHandler;
        }
    }

    serverSocket._close();
    cache->clear();
}
