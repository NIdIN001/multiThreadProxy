#include "ClientSocketHandler.h"

ClientSocketHandler::ClientSocketHandler(TcpSocket *clientSocket, HttpProxy *proxy) :
        clientSocket(clientSocket), proxy(proxy) {
    this->runningThread = new std::thread(&ClientSocketHandler::run, this);
    runningThread->detach();
}

ClientSocketHandler::~ClientSocketHandler() = default;

bool ClientSocketHandler::parseRequest(char *request) {
    if (strstr(request, "GET") == request) {
        char *urlStart = strstr(request, "http://") + 7;

        char *url = new char[BUFFER_SIZE];
        int i;
        for (i = 0; urlStart[i] != ' '; i++) {
            url[i] = urlStart[i];
        }
        url[i] = '\0';

        proxy->gotNewRequest(this, url);
    } else if (strstr(request, "CONNECT") == request) {
        return false;
    }

    return true;

}

void ClientSocketHandler::run() {
    while (true) {
        if (isStop)
            return;

        char *buf = new char[MAX_CHUNK_SIZE];
        int length = clientSocket->_read(buf, MAX_CHUNK_SIZE);
        if (length <= 0) {
            proxy->closeSession(clientSocket);
            return;
        }

        if (!parseRequest(buf)) {
            proxy->closeSession(clientSocket);
            return;
        }

        if (hostSocket != nullptr) {
            length = hostSocket->_write(buf, length);
            if (length <= 0) {
                proxy->closeSession(hostSocket);
                return;
            }
        }
    }
}

void ClientSocketHandler::setHostSocket(TcpSocket *hostSocket) {
    this->hostSocket = hostSocket;
}

int ClientSocketHandler::getClientFd() {
    return clientSocket->fd;
}
