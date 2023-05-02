#include "SenderFiles.h"
#include <QDataStream>
#include <QByteArray>

SenderFiles::SenderFiles(LogQThread* loger, QObject* parent) {
	log_qthread   = loger;
    cfg_struct    = NULL;
    client_sender = NULL;
}

void SenderFiles::SetConfig(ConfigStruct* cfg) {
    cfg_struct = cfg;
}

void SenderFiles::StopSend() {
	flag_run = false;
}

void SenderFiles::resSendFile(bool res_send, const QQueue<QString>& files_queue) {
    this->client_run  = false;
    this->res_send    = res_send;
    this->files_queue = files_queue;

    delete client_sender;
    client_sender = NULL;
}

void SenderFiles::run() {
    flag_run = true;
    emit log_qthread->LogAdd(time_now(), tr("Запуск потока передачи файлов"));
    bool res_send = false;

    QRegExp re(cfg_struct->mask_files);
    QDirIterator it(cfg_struct->path_send, QDir::Files);
    if (it.hasNext()) {
        while (it.hasNext()) {
            QString file_name = it.next();
            QStringList list_name = file_name.split("/");
            
            if (list_name.size() > 0) {
                QString only_name = *list_name.rbegin();

                if (re.exactMatch(only_name))
                    files_queue.enqueue(file_name);
            }
        }
        emit log_qthread->LogAdd(time_now(), tr("Файлов на отправку: %1").arg(files_queue.size()));
        SendFiles();
    }
    else
        emit log_qthread->LogAdd(time_now(), tr("Нет файлов для передачи"));

    flag_run = false;
    emit EndSend();
}

void SenderFiles::SendFiles() {
    client_sender = new Client(log_qthread, this);
    connect(client_sender, &Client::returnRes, this, &SenderFiles::resSendFile);

    client_sender->requestNewConnection(cfg_struct->IP_server, cfg_struct->port.toInt());
    client_sender->setFileTransf(files_queue, cfg_struct->block_size.toInt());
    client_sender->start();
    client_run = true;

    while (client_run) { QThread::sleep(2); }
    if (res_send)
        emit log_qthread->LogAdd(time_now(), tr("Отправка файлов завершена"));
    else {
        emit log_qthread->LogAdd(time_now(), tr("Не было передано %1 файлов").arg(files_queue.size()));
        emit log_qthread->LogAdd(time_now(), tr("Повторная попытка передачи через 10 секунд"));
        QThread::sleep(10);
        SendFiles();
    }
}