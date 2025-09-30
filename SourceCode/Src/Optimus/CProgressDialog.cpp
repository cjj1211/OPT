#include "CProgressDialog.h"
#include "ui_CProgressDialog.h"
#include <QGraphicsDropShadowEffect>
#include <QMovie>

BEGIN_NX_NAMESPACE

CProgressDialog::CProgressDialog(Qt::WindowModality modality , QWidget *parent)
    : CDragableDialog(parent)
    , ui(new Ui::CProgressDialog())
{
    ui->setupUi(this);
    this->setWindowModality(modality);
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

    movie = new QMovie(":/Optimus/images/common/round-progress.gif");
    movie->setScaledSize(QSize(ui->progressLabel->width(), ui->progressLabel->width()));
    ui->progressLabel->setMovie(movie);
    movie->start();
}

CProgressDialog::~CProgressDialog()
{}

void CProgressDialog::setContent(const QString& content)
{
    ui->contentLabel->setText(content);
}

END_NX_NAMESPACE

