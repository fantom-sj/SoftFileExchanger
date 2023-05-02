#pragma once
#include <QTcpServer>
#include <QRandomGenerator>

#include "LogQThread.h"
#include "ServerThread.h"

#include <QStringList>
#include <QTcpServer>


class Server : public QTcpServer {
    Q_OBJECT

    public:
        Server(LogQThread* log_qthread, QObject* parent = nullptr);
        void setPathRecv(QString path_recv);

    protected:
        void incomingConnection(qintptr socketDescriptor) override;

    private slots:
        void endSrever();

    signals:
        void endServerSignal();

    private:
        QString    path_recv;
        LogQThread *log_qthread;
};

