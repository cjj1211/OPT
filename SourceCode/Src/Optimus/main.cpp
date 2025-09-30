/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "Optimus.h"
#include "NxSystemCfg/CSystemCfg.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QTextStream>
#include <QFontDatabase>
#include <QDebug>



int main(int argc, char* argv[])
{

//#ifdef _DEBUG
    CLogger::Instance().SetLogLevel(LogPriority::LOG_DEBUG);
    CLogger::Instance().EnableDebug(true);
//#endif

    LOG_INFO("Optimus APP ..........................start");
    QApplication a(argc, argv);
    qApp->setOrganizationName("NeuroXess");
    qApp->setOrganizationDomain("neuroxess.com");
    qApp->setApplicationName("OptimusClient");
   
    QFile file(":/Optimus/NxStyle.qss");
    if (file.exists())
    {
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&file);
        qApp->setStyleSheet(ts.readAll());
    }

    auto boldId = QFontDatabase::addApplicationFont(QStringLiteral(":/Optimus/SiYuanHeiTi/SourceHanSansCN-Bold.ttf"));
    auto boldFamilies = QFontDatabase::applicationFontFamilies(boldId);
    
    auto fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/Optimus/SiYuanHeiTi/SourceHanSansCN-Normal.ttf"));
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    

    QTranslator translator;
    if (systemConfig.GetSystemCfg().Common.Language == "zh-CN")
    {
        translator.load("..\\config\\Optimus_zh_CN.qm");
        QApplication::installTranslator(&translator);
    }

    if (boldFamilies.size() > 0)
    {
        QFont font;
        font.setFamily(boldFamilies[0]);//设置全局字体
        qApp->setFont(font);
    }

    Optimus optimus;
    //optimus.ChangeLanguage("zh-CN");
    optimus.show();
    
    LOG_INFO("Optimus APP ..........................finished");
    return a.exec();
}
