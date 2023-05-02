#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SoftFileExchanger.h"
#include <QThread>
#include <QDebug>
#include <QFileDialog>
#include <QNetworkInterface>

#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <chrono>

#include "LogQThread.h"
#include "SenderFiles.h"
#include "ConfigStruct.h"

#include "Client.h"
#include "Server.h"

using namespace std;

class SoftFileExchanger : public QMainWindow {
    Q_OBJECT

    public:
        SoftFileExchanger(QWidget *parent = nullptr);
        ~SoftFileExchanger();

    private slots:
        void ClickButtonSenderStart();
        void ClickButtonServerStart();

        void ClickButtonPathSend();
        void ClickButtonPathRecv();

        void EndSender();

        void EndServer();

    private:
        Ui::SoftFileExchangerClass ui;
        ConfigStruct cfg;

        LogQThread  *log_qthread;
        SenderFiles *sender_qthread;

        Server *server;

        bool sender_run = false;
        bool server_run = false;
    
        void LoadConfig();
        void UpdateConfig();
        bool PathRecvCheck();
        bool PathSendCheck();
};
