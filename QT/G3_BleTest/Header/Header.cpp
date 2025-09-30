/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 :cuijunjie junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "Header.h"
#include "../../NxMessage/TransInformation.h"
#include <QFontDatabase>
#include <QTimer>
#include <QBitmap>
#include <QPainter>
BEGIN_NX_NAMESPACE

Header::Header(Patient patient, GameInfo gameInfo,QWidget* parent)
    :QWidget(parent)
{

    ui = std::make_unique<Ui::Header>();
    auto boldId = QFontDatabase::addApplicationFont(QStringLiteral("./SiYuanHeiTi/SourceHanSansCN-Bold.ttf"));
    auto boldFamilies = QFontDatabase::applicationFontFamilies(boldId);

    auto fontId = QFontDatabase::addApplicationFont(QStringLiteral("./SiYuanHeiTi/SourceHanSansCN-Normal.ttf"));
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);

    QFont font_bold(boldFamilies.at(0), 12, QFont::Bold);
    QFont font_normal(fontFamilies.at(0), 12);
    ui->setupUi(this);
    ui->userName->setText(QString::fromStdString(patient.PatientName));
    ui->userName->setFont(font_bold);
    ui->time->setFont(font_bold);
    ui->type->setFont(font_bold);
    showAvatar(QString::fromStdString(patient.Avatar));
    ui->cutdown->setFont(font_bold);
    setFixedHeight(56);
    connect(ui->shutdownBtn, &QPushButton::clicked, this, &Header::slotShutDown);
    ui->time->setText(QString::fromLocal8Bit("第%1次").arg(1));
}

Header::~Header()
{

}


void Header::changeWarnLabel(bool isWarning)
{
	
}

void Header::showAvatar(const QString& avatar) const
{
	if (avatar.isEmpty())
	{
        ui->avatarLabel->setPixmap(QPixmap(":/Optimus/images/common/avatar 30x30.png"));
	}
	else
	{
        QByteArray ba = QByteArray::fromBase64(avatar.toLatin1());

        QImage img;
        img.loadFromData(ba, "jpg");
        ba.clear();
        auto showImg = QPixmap::fromImage(img.scaled(ui->avatarLabel->width(), ui->avatarLabel->height(), Qt::KeepAspectRatio));

        const auto radius = ui->avatarLabel->width() / 2;
        const QSize size(ui->avatarLabel->width(), ui->avatarLabel->height());
        QBitmap mask(size);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
        painter.setBrush(QColor(220, 0, 0));
        painter.drawEllipse(QPoint(radius, radius), radius, radius);
        showImg.setMask(mask);
        ui->avatarLabel->setPixmap(showImg);
	}

}

void Header::slotShutDown()
{
    emit shutDown();
}

void Header::slotUpdateCutDown(int minutes, int seconds)
{
        ui->cutdown->setText(QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
}

void Header::changeActionNumber(int number)
{
    ui->time->setText(QString::fromLocal8Bit("第%1次").arg(number));
}

END_NX_NAMESPACE


