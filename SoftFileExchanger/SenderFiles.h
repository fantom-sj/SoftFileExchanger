#pragma once
#include <QTCore>
#include <QRegExp>
#include <QThread>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QPushButton>

#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <filesystem>

#include "LogQThread.h"
#include "ConfigStruct.h"
#include "Client.h"

class SenderFiles : public QThread {
	Q_OBJECT
	public:
		explicit SenderFiles(LogQThread* loger, QObject* parent = nullptr);

	public slots:
		void SetConfig(ConfigStruct* cfg);
		void StopSend();
		void resSendFile(bool res_send, const QQueue<QString>& files_queue);

	signals:
		void EndSend();

	private:
		QQueue<QString> files_queue;

		LogQThread   *log_qthread;
		ConfigStruct *cfg_struct;

		bool flag_run   = false;
		bool res_send   = false;
		bool client_run = false;

		Client* client_sender;
		void run();
		void SendFiles();
};

