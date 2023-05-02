#pragma once
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTcpSocket> 
#include <QFile>

#include "LogQThread.h"

class Client : public QThread {
    Q_OBJECT

    public:
        Client(LogQThread* log_qthread, QObject* parent = nullptr);
        ~Client();

        bool quit;

        void requestNewConnection(const QString& hostName, quint16 port);
        void setFileTransf(const QQueue<QString>& files_queue, quint32 block_size);
        void run() override;

    signals:
        void returnRes(bool res_send, const QQueue<QString>& files_queue);

    private:
        LogQThread     *log_qthread;
        QString         host_ip;
        quint16         port;
        quint32         block_size;
        QQueue<QString> files_queue;

        QMutex          mutex;
        QWaitCondition  cond;
};

