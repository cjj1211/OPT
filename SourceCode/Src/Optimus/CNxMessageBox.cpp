#include "CNxMessageBox.h"
#include <QGraphicsDropShadowEffect>

BEGIN_NX_NAMESPACE

CNxMessageBox::CNxMessageBox(const QString& title, const QString& info, QWidget *parent)
    : CDragableDialog(parent)
    , ui(new Ui::CNxMessageBox())
    , confirmed(false)
{
    ui->setupUi(this);

    // 设置无边框和背景透明
    setAttribute(Qt::WA_TranslucentBackground, true);
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::FramelessWindowHint;
    setWindowFlags(flags);

    // 设置阴影效果
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(Qt::black);
    shadowEffect->setBlurRadius(15);
    this->setGraphicsEffect(shadowEffect);

    ui->titleLabel->setText(title);
    ui->contentLabel->setText(info);

    connect(ui->confirmBtn, SIGNAL(clicked()), SLOT(slotConfirmed()));
}

CNxMessageBox::~CNxMessageBox()
{

}

void CNxMessageBox::slotConfirmed()
{
    confirmed = true;
    close();
}

END_NX_NAMESPACE