#pragma once
#include <QtCore>
#include <QThread>
#include <QtWidgets/QTextEdit>
#include <fstream>
#include <chrono>

#define time_now() \
    (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))

#define LogTab 0
#define LogInfo 1

class LogQThread: public QThread {
    Q_OBJECT

    public:
        explicit LogQThread(QTextEdit* log_edit, QString log_file_name, QObject *parent = nullptr);
        ~LogQThread();

    public slots:
        void LogAdd(time_t log_time, QString log_rec);

    private:
        QQueue<QPair<time_t, QString>> log_queue;

        std::fstream file_log;
        QTextEdit    *log_ui_edit;

        void run();
};

