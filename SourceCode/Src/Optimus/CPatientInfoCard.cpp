#include "CPatientInfoCard.h"
#include "NxEntity/Patient.h"
#include "NxEntity/IdentityInfo.h"
#include "NxAppService/CPatientAdminService.h"
#include <QBuffer>
#include <QBitmap>
#include <QPainter>
#include <QPalette>
#include <QMouseEvent>

BEGIN_NX_NAMESPACE

CPatientInfoCard::CPatientInfoCard(CPatientAdminService* paService, const QString& patientUid, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CPatientInfoCard())
    , service(paService)
    , curPatientUid(patientUid)
    , isSelected(false)
{
    ui->setupUi(this);
    
    patientInfo = new Patient();
    
    updatePatientInfo();
}

CPatientInfoCard::~CPatientInfoCard()
{
    delete patientInfo;
}

void CPatientInfoCard::showAvatar() const
{
    IdentityInfo identityInfo;
    service->getPatientIdentifyInfo(QString::fromStdString(patientInfo->IdentifyFK), identityInfo);
    if (!identityInfo.Avatar.empty()) {
        QByteArray ba;
        for (const char i : identityInfo.Avatar)
        {
            ba.append(i);
        }

        QImage img;
        img.loadFromData(ba, "jpg");
        ba.clear();
        constexpr auto borderWidth = 2;
        auto showImg = QPixmap::fromImage(img.scaled(ui->avator->width() - borderWidth * 2 , ui->avator->height() - borderWidth * 2, Qt::KeepAspectRatio));
        const auto radius = ui->avator->width() / 2 - borderWidth;
        const QSize size(ui->avator->width() - borderWidth * 2, ui->avator->height() - borderWidth * 2);
        QBitmap mask(size);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
        painter.setBrush(QColor(0, 0, 0));
        painter.drawEllipse(QPoint(radius, radius), radius, radius);
        showImg.setMask(mask);
        ui->avator->setPixmap(showImg);
    }
}

void CPatientInfoCard::setSelected(bool selected)
{
    isSelected = selected;
    QString style = "";
    if (isSelected) {
        style = "#mainLayout {\
                    background-color: #283C82;\
                    border-radius: 4px;\
                    border: 1px solid #1FE9F4;\
                 }";
        
    }
    else {
        style = "#mainLayout {\
                    background-color: #283C82;\
                    border-radius: 4px;\
                    border: 1px solid gray;\
                 }";
    }
    ui->mainLayout->setStyleSheet(style);
}

QString CPatientInfoCard::patientUid() const
{ 
    if (patientInfo == nullptr)  return "";
    return QString::fromStdString(patientInfo->UID); 
}

void CPatientInfoCard::mousePressEvent(QMouseEvent* event)
{
    setSelected(true);
    emit signalPressed(QString::fromStdString(patientInfo->UID));
}

void CPatientInfoCard::mouseDoubleClickEvent(QMouseEvent* event)
{
    setSelected(true);

    if (ui->avator->geometry().contains(event->pos())) {
        emit signalDoubleClicked(QString::fromStdString(patientInfo->UID));
    }
    else {
        emit signalJumpToTrainPage(QString::fromStdString(patientInfo->UID));
    }
}

void CPatientInfoCard::updatePatientInfo() const
{
    if (!service->getPatientByUid(curPatientUid, *patientInfo)) {
        return;
    }

    ui->ID->setText(QString::fromStdString(patientInfo->CaseID));
    ui->patientName->setText(QString::fromStdString(patientInfo->PatientName));
    auto birthday = QString::fromStdString(patientInfo->Birthday).split(' ')[0];
    auto age = QDate::currentDate().year() - QDate::fromString(birthday, "yyyy-MM-dd").year();
    ui->age->setText(QString::number(age));
    QString illstage;
    if (patientInfo->IllType == UNKOWN) {
        illstage = tr("Unselected");
    }
    else if (patientInfo->IllType == ALS) {
        const QString levels[] = { "1", "2", "2", "3", "4", "4", "5" };
        const QString suffix[] = { "", "A", "B", "", "A", "B", "" };
        illstage = tr("ALS") + levels[patientInfo->IllStage] + tr("Phase") + suffix[patientInfo->IllStage];
    }
    else if (patientInfo->IllType == Seizures) {
        QString levels[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };
        illstage = tr("Seizures") + levels[patientInfo->IllStage] + tr("Level");
    }
    else if (patientInfo->IllType == Depression) {
        QString levels[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };
        illstage = tr("Depression") + levels[patientInfo->IllStage] + tr("Level");
    }
    else if (patientInfo->IllType == SCI) {
        QString levels[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I" };
        illstage = tr("SCI") + levels[patientInfo->IllStage] + tr("Level");
    }
    else if (patientInfo->IllType == PD) {
        QString levels[] = { "0", "1", "1.5", "2", "2.5", "3", "4", "5" };
        illstage = tr("PD") + levels[patientInfo->IllStage] + tr("Phase");
    }
    else if (patientInfo->IllType == Blindness) {
        QString levels[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
        illstage = tr("Blindness") + levels[patientInfo->IllStage] + tr("Level");
    }
    ui->stage->setText(illstage);

    IdentityInfo identity;
    service->getPatientIdentifyInfo(QString::fromStdString(patientInfo->IdentifyFK), identity);
    QString genderLabelStyle;
    if (patientInfo->Gender == male) {
        genderLabelStyle = "image: url(:/Optimus/images/common/male.svg);";
    }
    else {
        genderLabelStyle = "image: url(:/Optimus/images/common/female.svg);";
    }
    ui->gender->setStyleSheet(genderLabelStyle);
    if (identity.Avatar.empty())
    {

        QString style;
        if (patientInfo->Gender == male) {
            style = "QLabel { \
                                image: url(:/Optimus/images/common/man-avator.svg); \
                                background- color: #ffffff; \
                                border-radius: 60px; \
                                border: 2px solid gray; \
                            } ";
        }
        else {
            style = "QLabel { \
                                image: url(:/Optimus/images/common/woman-avator.svg); \
                                background- color: #ffffff; \
                                border-radius: 60px; \
                                border: 2px solid gray; \
                            } ";
        }
        ui->avator->setStyleSheet(style);
        
    }
    else {
        showAvatar();
    }
}

END_NX_NAMESPACE


