#include "CLoadingScreen.h"
#include "ui_CLoadingScreen.h"

CLoadingScreen::CLoadingScreen(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CLoadingScreen)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);  // 无边框
    setModal(true);  // 阻止用户操作其他窗口
    setWindowTitle(QString::fromLocal8Bit("升级！"));
   
    loadingMovie = new QMovie(QCoreApplication::applicationDirPath()+"/Loading.gif");
    ui->loading->setMovie(loadingMovie);
    loadingMovie->setScaledSize(ui->loading->size());  // 让 GIF 适应 QLabel 大小
    ui->uapdateStatus->setText((QString::fromLocal8Bit("文件传输中")));
    loadingMovie->start();
    setAttribute(Qt::WA_TranslucentBackground);  // 让整个窗口背景透明
}

CLoadingScreen::~CLoadingScreen()
{
    delete ui;
}

void CLoadingScreen::setStatus(QString status)
{
    ui->uapdateStatus->setText((status));
}

