#include "LogQThread.h"

LogQThread::LogQThread(QTextEdit* log_edit, QString log_file_name, QObject* parent) : 
	QThread(parent) {

	log_ui_edit = log_edit;
	file_log.open(log_file_name.toStdString(), std::ios_base::app);
}

LogQThread::~LogQThread() {
	//mutex.lock();
	file_log.close();
	//cond.wakeOne();
	//mutex.unlock();
	//wait(10);
}

void LogQThread::LogAdd(time_t log_time, QString log_rec) {
	log_queue.enqueue({ log_time, log_rec });
}

void LogQThread::run() {
	while (true) {
		QQueue logi = log_queue;

		if (!logi.empty()) {
			QPair<time_t, QString> log_data = log_queue.dequeue();

			QString log_str;
			if (!log_data.first == LogTab) {
				struct tm  tstruct;
				char       buf[80];
				tstruct = *localtime(&log_data.first);
				strftime(buf, sizeof(buf), "%d-%m-%Y %X ", &tstruct);
				log_str = tr(buf) + log_data.second + "\n";
			}
			else if (log_data.first == LogTab) {
				QString tab = tr("\t          ");
				log_str = tab + log_data.second + "\n";
			}
			else if (log_data.first == LogInfo) {
				log_str = tr("Информация: ") + log_data.second + "\n";
			}
			else {
				log_str = log_data.second + "\n";
			}

			QTextCursor textCursor = log_ui_edit->textCursor();
			textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);
			log_ui_edit->setTextCursor(textCursor);

			log_ui_edit->insertPlainText(log_str);
			file_log << log_str.toStdString();
		}
	}
}



