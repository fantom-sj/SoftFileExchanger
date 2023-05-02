#include "SoftFileExchanger.h"
#include <QtWidgets/QApplication>
#include <QtCore5Compat/QTextCodec>


int main(int argc, char *argv[]) {
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("Windows-1251"));

    QApplication app(argc, argv);
    SoftFileExchanger main_window;
    
    main_window.show();
    return app.exec();
}
