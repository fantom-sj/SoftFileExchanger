#include "ServerThread.h"

ServerThread::ServerThread(qintptr socketDescriptor, LogQThread* log_qthread,
    QString path_recv, QObject* parent)
    : QThread(parent), socketDescriptor(socketDescriptor), datablockSize(0) {
    
    this->path_recv   = path_recv;
    this->log_qthread = log_qthread;
}

void ServerThread::run() {
    qint16 TimeOut = 5000;
    QTcpSocket tcpSocket;
    
    if (!tcpSocket.setSocketDescriptor(socketDescriptor)) {
        emit log_qthread->LogAdd(time_now(),
            tr("Ошибка при приёме файлов: %1").arg(tcpSocket.errorString()));
        return;
    }
    emit log_qthread->LogAdd(time_now(), 
        tr("Сервер установил связь c: %1").arg(tcpSocket.localAddress().toString()));

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5);
    out << tr("Сервер готов принимать данные!");
    tcpSocket.write(block);

    QDataStream in(&tcpSocket);
    in.setVersion(QDataStream::Qt_6_5);
    
    if (!tcpSocket.waitForReadyRead(TimeOut)) {
        emit log_qthread->LogAdd(time_now(),
            tr("Ошибка при приёме файлов: %1").arg(tcpSocket.errorString()));
        return;
    }
    in.startTransaction();

    qint64 count_file;
    in >> count_file;
    qDebug() << tr("Сервер должен принять: %1").arg(count_file);
    qDebug() << tr("Директория: %1").arg(path_recv);

    for (qint16 i = 0; i < count_file; i++) {
        if (tcpSocket.bytesAvailable() < sizeof(quint32))
            tcpSocket.waitForReadyRead(TimeOut);

        qDebug() << tr("Данных в ожидании на сервере") << tcpSocket.bytesAvailable();
        
        if (datablockSize == 0) {
            if (tcpSocket.bytesAvailable() < sizeof(quint32))
                return;
            in >> datablockSize;
        }
        if (tcpSocket.bytesAvailable() < datablockSize)
            return;

        if (tcpSocket.bytesAvailable() < datablockSize)
            tcpSocket.waitForReadyRead(TimeOut);

        QString fileName;
        qint64 file_size;

        in >> fileName;
        in >> file_size;
        qDebug() << tr("Принимаем файл %1").arg(fileName);
        qDebug() << tr("Размер файла %1").arg(file_size);

        fileName = fileName.section("/", -1);
        emit log_qthread->LogAdd(time_now(), tr("Принимаем файл: %1").arg(fileName));
        QFile target(path_recv + "/" + fileName);

        if (!target.open(QIODevice::WriteOnly)) {
            emit log_qthread->LogAdd(time_now(),
                tr("Ошибка при приёме файлов: не удалось открыть файл %1").arg(target.fileName()));
            tcpSocket.abort();
            return;
        }

        quint64 read_size = 0,          // Необходимо прочитать в текущую итерацию
                left_size = file_size,  // Осталось прочитать всего
                all_read  = 0;          // Всего было прочитано

        bool res = false;
        forever{
            if (!tcpSocket.waitForReadyRead(1000)) {
                emit log_qthread->LogAdd(time_now(),
                    tr("Ошибка при приёме файлов: ").arg(tcpSocket.errorString()));
                target.close();
                target.remove();

            }

            if (left_size > tcpSocket.bytesAvailable())
                read_size = tcpSocket.bytesAvailable();
            else
                read_size = left_size;
            
            qDebug() << "Данных на сервере" << tcpSocket.bytesAvailable();

            QByteArray line = tcpSocket.read(read_size);

            qDebug() << "Должны прочитать" << read_size;
            qDebug() << "Прочитали" << line.size();

            target.write(line);
            target.flush();
            target.waitForBytesWritten(-1);

            left_size -= line.size();
            all_read += line.size();
            

            if (all_read == file_size) {
                res = true;
                break;
            }   
        }

        if (res){
            qDebug() << tr("Файл принят");
            emit log_qthread->LogAdd(time_now(), tr("Файл %1 успешно принят").arg(fileName));
            target.close();
        }
        
        datablockSize = 0;
    }

    emit log_qthread->LogAdd(time_now(), tr("Сервер завершил приём файлов"));

    tcpSocket.disconnectFromHost();
    if (tcpSocket.state() == QAbstractSocket::UnconnectedState
        || tcpSocket.waitForDisconnected(10000)) {
        qDebug("Сервер завершил соединение");
    }
}