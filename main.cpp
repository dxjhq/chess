#include "Chess.h"
#include <QtWidgets/QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序编码
    app.setApplicationName("鲨鱼象棋");
    app.setApplicationVersion("V1.8.0");
    app.setOrganizationName("Chess Game");
    
    Chess window;
    window.show();
    return app.exec();
}
