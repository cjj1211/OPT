#include "CLoadingScreen.h"
#include "QTimer"
#include "NxSystemCfg/CSystemCfg.h"
#include <QMovie>
BEGIN_NX_NAMESPACE

CLoadingScreen::CLoadingScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CLoadingScreen())
{
    ui->setupUi(this);
    // 设置显示格式为百分比

    QMovie* movie = new QMovie(":/NXMotionTraining/images/common/normalvideo.gif");
    movie->setScaledSize(QSize(ui->giflabel->width(), ui->giflabel->height()));
    ui->giflabel->setMovie(movie);
    // 开始播放动画
    movie->start();
    connect(ui->progressBar, &QProgressBar::valueChanged, this, &CLoadingScreen::checkProgress);
    // 创建一个定时器模拟任务进度更新
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CLoadingScreen::updateProgress);
    timer->start(1000); // 每100毫秒更新一次进度
}
void CLoadingScreen::updateProgress()
{

    if (progress > 100) {
        progress = 0; // 重置进度为0
    }
    else {
        progress +=40; // 增加进度
    }

    // 更新进度条的值
    ui->progressBar->setValue(progress);
    ui->progressBar->setFormat("%p%");
    // 显示进度条的文本
    ui->progressBar->setTextVisible(true);

}
void CLoadingScreen::checkProgress(int value)
{

   /* if (value>=100)
    {
        emit signalShowLogin();

    }*/
}

END_NX_NAMESPACE