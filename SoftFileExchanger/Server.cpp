#include "Server.h"

Server::Server(LogQThread* log_qthread, QObject* parent) : QTcpServer(parent) {
    this->log_qthread = log_qthread;
}

void Server::incomingConnection(qintptr socketDescriptor) {
    ServerThread* thread = new ServerThread(socketDescriptor, log_qthread, path_recv, this);
    connect(thread, &ServerThread::finished, thread, &ServerThread::deleteLater);
    thread->start();
}

void Server::setPathRecv(QString path_recv) {
    this->path_recv = path_recv;
}

void Server::endSrever() {
    emit endServerSignal();
}