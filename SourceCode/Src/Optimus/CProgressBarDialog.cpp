#include "CProgressBarDialog.h"
#include <QGraphicsDropShadowEffect>

CProgressBarDialog::CProgressBarDialog(QWidget *parent)
    : CDragableDialog(parent)
    , ui(new Ui::CProgressBarDialog())
{
    ui->setupUi(this);

    // 设置无边框和背景透明
    setAttribute(Qt::WA_TranslucentBackground, true);
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::FramelessWindowHint;
    setWindowFlags(flags);

    // 设置阴影效果
    const auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(Qt::black);
    shadowEffect->setBlurRadius(15);
    this->setGraphicsEffect(shadowEffect);
}

CProgressBarDialog::~CProgressBarDialog() = default;

void CProgressBarDialog::slotUpdateProgress(int progress)
{
    ui->progressBar->setValue(progress);
}

void CProgressBarDialog::slotUpdateContentText(QString text)
{
    ui->label->setText(text);
}
