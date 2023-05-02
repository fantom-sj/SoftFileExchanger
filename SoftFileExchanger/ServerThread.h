#pragma once
#include <QThread>
#include <QTcpSocket>
#include <QFile>

#include "LogQThread.h"

class ServerThread : public QThread {
    Q_OBJECT

    public:
        ServerThread(qintptr socketDescriptor, LogQThread* log_qthread, 
            QString path_recv, QObject* parent);
        void run() override;

    signals:
        void endServerThread();

    private:
        qintptr     socketDescriptor;
        QString     path_recv;
        quint32     datablockSize;
        LogQThread *log_qthread;
};

