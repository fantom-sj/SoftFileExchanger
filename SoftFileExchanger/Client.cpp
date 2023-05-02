#include "Client.h"

Client::Client(LogQThread* log_qthread, QObject* parent) : QThread(parent), quit(false) {
    this->log_qthread = log_qthread;
}

Client::~Client() {
    mutex.lock();
    quit = true;
    cond.wakeOne();
    mutex.unlock();
    wait();
}

void Client::requestNewConnection(const QString& host_ip, quint16 port) {
    QMutexLocker locker(&mutex);

    this->host_ip  = host_ip;
    this->port     = port;

    if (!isRunning())
        start();
    else
        cond.wakeOne();
}

void Client::setFileTransf(const QQueue<QString>& files_queue, quint32 block_size) {
    QMutexLocker locker(&mutex);

    this->files_queue = files_queue;
    this->block_size  = block_size;

    if (!isRunning())
        start();
    else
        cond.wakeOne();
}

void Client::run() {
    mutex.lock();
    QString         server_ip   = host_ip;
    quint16         server_port = port;
    QQueue<QString> Qfiles      = files_queue;
    mutex.unlock();

    emit log_qthread->LogAdd(time_now(), 
        tr("Установлена связь с сервером: %1, порт: %2").arg(server_ip).arg(server_port));

    qint16 TimeOut = 5000;

    QTcpSocket socket;
    socket.connectToHost(server_ip, server_port);

    if (!socket.waitForConnected(TimeOut)) {
        emit log_qthread->LogAdd(time_now(), 
            tr("Ошибка при отправке файлов: %1").arg(socket.errorString()));
        emit returnRes(false, Qfiles);
        return;
    }

    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_6_5);
    QString server_response;

    bool flag_server = false;
    do {
        if (!socket.waitForReadyRead(TimeOut)) {
            emit log_qthread->LogAdd(time_now(),
                tr("Ошибка при отправке файлов: %1").arg(socket.errorString()));

            emit returnRes(false, Qfiles);
            return;
        }

        in.startTransaction();
        in >> server_response;

        if (!server_response.isEmpty())
            flag_server = true;

    } while (!in.commitTransaction());
    emit log_qthread->LogAdd(time_now(), server_response);

    while (!quit) {
        mutex.lock();

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_5);
        out << Qfiles.size();
        socket.write(block);

        socket.waitForBytesWritten(TimeOut);

        bool first = true;
        while (!Qfiles.isEmpty()) {
            QString file_name = Qfiles.dequeue();
            
            if (flag_server) {
                QFile file;
                if (file_name.isEmpty()) {
                    emit returnRes(false, Qfiles);
                }
                else {
                    file.setFileName(file_name);
                    if (!file.open(QIODevice::ReadOnly)) {
                        emit log_qthread->LogAdd(time_now(),
                            tr("Ошибка при отправке файлов: не удалось открыть файл %1")
                            .arg(file.errorString()));
                        emit returnRes(false, Qfiles);
                    }
                }

                emit log_qthread->LogAdd(LogTab, tr("Передаём файл: %1").arg(file_name));

                QByteArray file_block;
                QDataStream file_out(&file_block, QIODevice::WriteOnly);
                file_out.setVersion(QDataStream::Qt_6_5);

                file_out << (quint32)0 << file.fileName() << file.size();
                file_out.device()->seek(0);
                file_out << (quint32)(file_block.size() - sizeof(quint32));

                socket.write(file_block);
                socket.waitForBytesWritten(TimeOut);
                file_block.clear();

                QByteArray file_data;
                while (!file.atEnd()) {
                    file_data.append(file.read(block_size));
                    qint64 y = socket.write(file_data);

                    if (socket.waitForBytesWritten(TimeOut)) {
                        qDebug() << "Данные записаны в сокет: " << y;
                    }
                    else {
                        emit log_qthread->LogAdd(time_now(), 
                            tr("Ошибка при отправке файлов: сбой при передаче"));

                        cond.wait(&mutex);
                        server_ip = host_ip;
                        server_port = port;
                        mutex.unlock();
                        file.close();

                        emit returnRes(false, Qfiles);
                        return;
                    }

                    file_data.clear();
                }

                file.close();
                emit log_qthread->LogAdd(time_now(), tr("Файл успешно передан"));
            }
        }
        emit returnRes(true, Qfiles);

        cond.wait(&mutex);
        server_ip   = host_ip;
        server_port = port;
        mutex.unlock();
    }

    emit log_qthread->LogAdd(time_now(), tr("Процесс передачи файлов завершён"));
}