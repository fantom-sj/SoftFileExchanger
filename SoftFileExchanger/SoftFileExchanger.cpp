#include "SoftFileExchanger.h"

SoftFileExchanger::SoftFileExchanger(QWidget *parent) : QMainWindow(parent) {
    ui.setupUi(this);

    // Поток для ведения логов
    log_qthread = new LogQThread(ui.log_data, tr("LogData.txt"), this);
    connect(log_qthread, &LogQThread::finished, log_qthread, &QObject::deleteLater);
    log_qthread->start();

    emit log_qthread->LogAdd(time_now(), tr("Запуск программы обмена файлами"));

    // Поток для отправки данных
    sender_qthread = new SenderFiles(log_qthread, this);
    connect(sender_qthread, &SenderFiles::EndSend, this, &SoftFileExchanger::EndSender);
    connect(sender_qthread, &SenderFiles::finished, sender_qthread, &QObject::deleteLater);

    // События нажатия кнопок
    connect(ui.client_start_button, SIGNAL(released()), this, SLOT(ClickButtonSenderStart()));
    connect(ui.server_start_button, SIGNAL(released()), this, SLOT(ClickButtonServerStart()));

    connect(ui.path_send_button, SIGNAL(released()), this, SLOT(ClickButtonPathSend()));
    connect(ui.path_recv_button, SIGNAL(released()), this, SLOT(ClickButtonPathRecv()));

    LoadConfig();
    PathRecvCheck();
    PathSendCheck();
}

SoftFileExchanger::~SoftFileExchanger() {

}

void SoftFileExchanger::LoadConfig() {
    string file_config_name = "config.txt";
    ifstream file_config(file_config_name);

    string parametr;
    if (file_config) {
        emit log_qthread->LogAdd(time_now(), tr("Конфигурационный файл найден и открыт"));
        emit log_qthread->LogAdd(time_now(), tr("Чтение конфигурации"));

        while (getline(file_config, parametr)) {
            string parametr_name, parametr_data;
            stringstream parametr_stream(parametr);
            getline(parametr_stream, parametr_name, '=');
            getline(parametr_stream, parametr_data, '=');

            emit log_qthread->LogAdd(LogTab, tr(parametr_name.c_str()) +
                                             " = " + QString(parametr_data.c_str()));

            if (parametr_name == "path_send") {
                ui.path_send_edit->setText(QString(parametr_data.c_str()));
            }
            else if (parametr_name == "path_recv") {
                ui.path_recv_edit->setText(tr(parametr_data.c_str()));
            }
            else if (parametr_name == "IP_server") {
                ui.IP_edit->setText(tr(parametr_data.c_str()));
            }
            else if (parametr_name == "port") {
                ui.port_edit->setText(tr(parametr_data.c_str()));
            }
            else if (parametr_name == "mask_files") {
                ui.file_maska_edit->setText(tr(parametr_data.c_str()));
            }
            else if (parametr_name == "block_size") {
                ui.block_size->setText(tr(parametr_data.c_str()));
            }
            else
                continue;
        }

        file_config.close();
    }
    else {
        file_config.close();

        emit log_qthread->LogAdd(time_now(), tr("Файл конфигурации не найден"));
        emit log_qthread->LogAdd(time_now(), tr("Используется конфигурация по-умолчанию"));
        
        QString path_send  = "files_to_send";
        QString path_recv  = "received_files";
        QString IP_server  = "192.168.0.1";
        QString port       = "6060";
        QString mask_files = "file_.*.txt";
        QString block_size = "120";

        ui.path_send_edit->setText(path_send);
        ui.path_recv_edit->setText(path_recv);
        ui.IP_edit->setText(IP_server);
        ui.port_edit->setText(port);
        ui.file_maska_edit->setText(mask_files);
        ui.block_size->setText(block_size);

        emit log_qthread->LogAdd(LogTab, "path_send = "  + path_send);
        emit log_qthread->LogAdd(LogTab, "path_recv = "  + path_recv);
        emit log_qthread->LogAdd(LogTab, "IP_server = "  + IP_server);
        emit log_qthread->LogAdd(LogTab, "port = "       + port);
        emit log_qthread->LogAdd(LogTab, "mask_files = " + mask_files);
        emit log_qthread->LogAdd(LogTab, "block_size = " + block_size);

        fstream file_config_new;
        file_config_new.open(file_config_name, ios_base::out);
        file_config_new << "path_send="  << path_send.toStdString()  << endl;
        file_config_new << "path_recv="  << path_recv.toStdString()  << endl;
        file_config_new << "IP_server="  << IP_server.toStdString()  << endl;
        file_config_new << "port="       << port.toStdString()       << endl;
        file_config_new << "mask_files=" << mask_files.toStdString() << endl;
        file_config_new << "block_size=" << block_size.toStdString();

        file_config_new.close();
    }
}

void SoftFileExchanger::UpdateConfig() {
    string file_config_name = "config.txt";

    cfg.path_send  = ui.path_send_edit->text();
    cfg.path_recv  = ui.path_recv_edit->text();
    cfg.IP_server  = ui.IP_edit->text();
    cfg.port       = ui.port_edit->text();
    cfg.mask_files = ui.file_maska_edit->text();
    cfg.block_size = ui.block_size->text();

    fstream file_config;
    file_config.open(file_config_name, ios_base::out);
    file_config << "path_send="  << cfg.path_send.toStdString()  << endl;
    file_config << "path_recv="  << cfg.path_recv.toStdString()  << endl;
    file_config << "IP_server="  << cfg.IP_server.toStdString()  << endl;
    file_config << "port="       << cfg.port.toStdString()       << endl;
    file_config << "mask_files=" << cfg.mask_files.toStdString() << endl;
    file_config << "block_size=" << cfg.block_size.toStdString();

    file_config.close();
}

bool SoftFileExchanger::PathRecvCheck() {
    filesystem::path path_received_files(ui.path_recv_edit->text().toStdWString());

    if (!filesystem::exists(path_received_files)) {
        emit log_qthread->LogAdd(time_now(), tr("Директории для принимаемых файлов не существует"));

        try {
            bool res = filesystem::create_directories(path_received_files);
            if (res)
                emit log_qthread->LogAdd(time_now(), tr("Директории для принимаемых файлов успешно создана"));
            else
                throw;

            return true;
        }
        catch (const filesystem::filesystem_error) {
            emit log_qthread->LogAdd(time_now(), tr("Ошибка создания директории для принимаемых файлов"));
            emit log_qthread->LogAdd(time_now(), tr("Измените директорию и повторите попытку!"));
            return false;
        }
    }
    else
        return true;
}

bool SoftFileExchanger::PathSendCheck() {
    filesystem::path path_files_to_send(ui.path_send_edit->text().toStdWString());

    if (!filesystem::exists(path_files_to_send)) {
        emit log_qthread->LogAdd(time_now(), tr("Директории для передаваемых файлов не существует"));

        try {
            bool res = filesystem::create_directories(path_files_to_send);
            if (res)
                emit log_qthread->LogAdd(time_now(), tr("Директории для передаваемых файлов успешно создана"));
            else
                throw;

            return true;
        }
        catch (const filesystem::filesystem_error) {
            emit log_qthread->LogAdd(time_now(), tr("Ошибка создания директории для передаваемых файлов"));
            emit log_qthread->LogAdd(time_now(), tr("Измените директорию и повторите попытку!"));
            return false;
        }
    }
    else
        return true;
}

void SoftFileExchanger::ClickButtonPathSend() {
    QString path_send = QFileDialog::getExistingDirectory(this, tr("Выбрать папку для передаваемых файлов"), "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (path_send.size() > 0)
        ui.path_send_edit->setText(path_send);
}

void SoftFileExchanger::ClickButtonPathRecv() {
    QString path_recv = QFileDialog::getExistingDirectory(this, tr("Выбрать папку для передаваемых файлов"), "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (path_recv.size() > 0)
        ui.path_recv_edit->setText(path_recv);
}

void SoftFileExchanger::ClickButtonSenderStart() {
    if (!sender_run) {
        UpdateConfig();

        emit log_qthread->LogAdd(time_now(), tr("Запуск передачи файлов на сервер"));

        if (!PathSendCheck()) {
            emit log_qthread->LogAdd(time_now(), tr("Директория с файлами для отправки не найдена"));
            return;
        }

        emit log_qthread->LogAdd(time_now(), tr("Директория с файлами для отправки успешно найдена"));

        emit sender_qthread->SetConfig(&cfg);

        sender_run = true;
        sender_qthread->start();
    }
    else {
        emit log_qthread->LogAdd(time_now(), tr("Процесс передачи файлов уже запущен"));
    }
}

void SoftFileExchanger::ClickButtonServerStart() {
    if (!server_run) {
        UpdateConfig();

        server = new Server(log_qthread, this);
        connect(server, &Server::endServerSignal, this, &SoftFileExchanger::EndServer);

        emit log_qthread->LogAdd(time_now(), tr("Запуск сервера для приема файлов"));

        if (!PathRecvCheck()) {
            emit log_qthread->LogAdd(time_now(), tr("Директория для принимаемых файлов не найдена"));
            return;
        }

        emit log_qthread->LogAdd(time_now(), tr("Директория для принимаемых файлов успешно найдена"));
        server->setPathRecv(cfg.path_recv);

        if (!server->listen(QHostAddress::Any, ui.port_edit->text().toInt())) {
            emit log_qthread->LogAdd(time_now(), tr("Не удалось запустить сервер: %1.")
                .arg(server->errorString()));
        }
        else {
            emit log_qthread->LogAdd(time_now(), tr("Сервер для приёма файлов запущен"));
        }

        server_run = true;
        emit log_qthread->LogAdd(time_now(), tr("Сервер работает на порту: %1").arg(server->serverPort()));
    }
    else {
        emit log_qthread->LogAdd(time_now(), tr("Сервер для приёма файлов уже запущен"));
    }
}

void SoftFileExchanger::EndSender() {
    sender_run = false;
}

void SoftFileExchanger::EndServer() {
    server_run = false;
}
