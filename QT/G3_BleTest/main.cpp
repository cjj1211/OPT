#include "G3_BleTest.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <CLogger.h>
#include <qdir.h>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <qprocess.h>
#include <QGuiApplication>
using namespace  NX;
int main(int argc, char *argv[])
{
#ifdef _DEBUG
	CLogger::Instance().EnableDebug(true);
#endif
 
    CLogger::Instance().EnableDebug(true);


    LOG_INFO("Optimus APP ..........................start");
    QApplication a(argc, argv);
    QFile file(QCoreApplication::applicationDirPath()+"/NxStyle.qss");
    if (file.exists())
    {
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&file);
        qApp->setStyleSheet(ts.readAll());
    }

    auto boldId = QFontDatabase::addApplicationFont(QStringLiteral("./SiYuanHeiTi/SourceHanSansCN-Bold.ttf"));
    auto boldFamilies = QFontDatabase::applicationFontFamilies(boldId);

    auto fontId = QFontDatabase::addApplicationFont(QStringLiteral("./SiYuanHeiTi/SourceHanSansCN-Normal.ttf"));
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (boldFamilies.size() > 0)
    {
        QFont font;
        font.setFamily(boldFamilies[0]);//设置全局字体
        qApp->setFont(font);
    }
    G3_BleTest w;
    return    a.exec();
}
