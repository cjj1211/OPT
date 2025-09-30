#include "CMotionTraining.h"
#include "NxSystemCfg/CSystemCfg.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QTranslator>
int main(int argc, char *argv[])
{
#ifdef _DEBUG
    CLogger::Instance().EnableDebug(true);
#endif
    LOG_INFO("NXMotionTraining APP ..........................start");
    QApplication a(argc, argv);
    QFile file("NxStyle.qss");
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
    QTranslator translator;
    auto cfg = systemConfig.GetSystemCfg().Common;
    if (systemConfig.GetSystemCfg().Common.Language == "zh-CN")
    {

        translator.load("..\\config\\NXMotionTraining_zh_CN.qm");
        QApplication::installTranslator(&translator);
    }  
    if (boldFamilies.size() > 0)
    {
        QFont font;
        font.setFamily(boldFamilies[0]);//设置全局字体
        qApp->setFont(font);
    }
    CMotionTraining w;
    w.show();
    return a.exec();
}
