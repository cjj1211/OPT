#include "CUserInfo.h"
#include <QBitmap>
#include <QBuffer>
#include <QPainter>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>
#include "NxAppService/CUserService.h"
#include "NxSystemCfg/CSystemCfg.h"
#include "NxEntity/Patient.h"
#include "Optimus/CRecordFace.h"

BEGIN_NX_NAMESPACE

constexpr float kFaceDetectThreshold = 0.98f;

CUserInfo::CUserInfo(CUserService* service_, QString loginName_, QWidget *parent)
    : CDragableDialog(parent)
    , ui(new Ui::CUserInfo)
    , userService(service_)
    , currentLoginName(std::move(loginName_))
    , recordStatus(RS_CAPTURE)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    ui->passwordStatus->setVisible(false);
    ui->passwordPrompt->setVisible(false);
    ui->usernameStatus->setVisible(false);
    ui->usernamePrompt->setVisible(false);
    ui->IsLock->setVisible(false);
    ui->capture->setEnabled(false);
    ui->confirm->setEnabled(false);

    if(!currentLoginName.isEmpty())
    {
        initWidget(currentLoginName);
    }

    connect(ui->close, &QPushButton::clicked, this, &CUserInfo::slotCloseWindow);
    connect(ui->closeBtn, &QPushButton::clicked, this, &CUserInfo::slotCloseWindow);
    connect(ui->Cancel, &QPushButton::clicked, this, &CUserInfo::slotCloseWindow);
    connect(ui->Save, &QPushButton::clicked, this, &CUserInfo::slotSaveUserInfo);
    connect(ui->recordFaceBtn, &QPushButton::clicked, this, &CUserInfo::slotSwitchToRecordFace);
    connect(ui->capture, SIGNAL(clicked()), this, SLOT(slotCapture()));
    connect(ui->confirm, SIGNAL(clicked()), this, SLOT(slotConfirm()));

    // 设置用户名输入限制
    QRegExp usernameRegex("[A-Za-z0-9]{1,20}");
    QValidator* usernameValidator = new QRegExpValidator(usernameRegex);
    ui->UserName->setValidator(usernameValidator);
    // 设置密码输入框属性
    ui->PasswordLine->setEchoMode(QLineEdit::Password);

    connect(ui->UserName, &QLineEdit::textEdited, this, [=] {
      ui->usernameStatus->setVisible(false);
      ui->usernamePrompt->setVisible(false);

      });
    connect(ui->PasswordLine, &QLineEdit::textEdited, this, [=] {
      ui->passwordPrompt->setVisible(false);
      ui->passwordStatus->setVisible(false);
      });

}

CUserInfo::~CUserInfo()
{
    releaseDetect();
};

void CUserInfo::initWidget(const QString& loginName)
{
    const auto identityInfos = userService->getUserByLoginName(loginName);
    if (!identityInfos.empty())
    {
        const auto userList = userService->getUserByIdentifyFK(QString::fromStdString(identityInfos[0].UID));
        currentUser = userList[0];
        currentIdentity = identityInfos[0];

        ui->PasswordLine->setText(QString::fromStdString(currentIdentity.Password));
        ui->UserName->setText(QString::fromStdString(currentIdentity.LoginName));
        ui->Name->setText(QString::fromStdString(currentUser.UserName));
        ui->UserType->setCurrentIndex(currentIdentity.Role);
        ui->Departments->setText(QString::fromStdString(currentUser.Departments));
        ui->UserName->setEnabled(false);
        if (currentIdentity.LoginCounts>=5)
        {
            ui->IsLock->setVisible(true);
            ui->IsLock->setChecked(true);
        }
        showAvatar();
    }
    else
    {
        currentIdentity.LoginCounts = 0;
        currentIdentity.InitialLogin = 0;
        currentIdentity.UID = QUuid::createUuid().toString().replace("{", "").replace("}", "").toStdString();
        currentIdentity.CreateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
        currentIdentity.UpdateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
        currentIdentity.IsDeleted = false;
        currentUser.UID = QUuid::createUuid().toString().replace("{", "").replace("}", "").toStdString();
        currentUser.CreateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
        currentUser.UpdateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
        currentUser.IsDeleted = false;
        ui->UserName->setEnabled(true);
        ui->IsLock->setVisible(false);
        ui->Departments->setText(QString::fromStdString(systemConfig.GetSystemCfg().Hospital.Department));
    }
}

void CUserInfo::slotSaveUserInfo()
{
	if (isCreate)
	{
        if (checkInfo())
        {
            if (currentIdentity.LoginName == "Sysuser" && ui->PasswordLine->text() != "admin123")
            {
                currentIdentity.InitialLogin = 1;
            }
            if (ui->IsLock->isChecked())
            {
                currentIdentity.LoginCounts = 6;
            }
            else
            {
                currentIdentity.LoginCounts = 0;
            }
            currentIdentity.Password = ui->PasswordLine->text().toStdString();
            currentIdentity.LoginName = ui->UserName->text().toStdString();
            currentIdentity.Role = ui->UserType->currentIndex();
          
            currentUser.UserName = ui->Name->text().toStdString();
            currentUser.IdentifyFK = currentIdentity.UID;
            currentUser.Departments = ui->Departments->text().toStdString();
       
            userService->CreateIdentityInfo(currentIdentity);
            userService->CreateUser(currentUser);
            emit initTableWidget();
            slotCloseWindow();
        }
		
	}
	else
	{
        if (checkInfo())
        {
            if (currentIdentity.LoginName == "Sysuser" && ui->PasswordLine->text() != "admin123")
            {
                currentIdentity.InitialLogin = 1;
            }
            if (ui->IsLock->isChecked())
            {
                currentIdentity.LoginCounts = 6;
            }
            else
            {
                currentIdentity.LoginCounts = 0;
            }
            currentIdentity.Password = ui->PasswordLine->text().toStdString();
            currentIdentity.LoginName = ui->UserName->text().toStdString();
            currentIdentity.Role = ui->UserType->currentIndex();
            currentIdentity.UpdateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
            currentUser.UserName = ui->Name->text().toStdString();
            currentUser.IdentifyFK = currentIdentity.UID;
            currentUser.Departments = ui->Departments->text().toStdString();
            currentUser.UpdateDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
            userService->UpdateIdentityInfo(currentIdentity);
            userService->UpdateUser(currentUser);
            emit initTableWidget();
            slotCloseWindow();
        }
	}

}
       
	
  


void CUserInfo::slotBroadcastFrame(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos,
    QVector<NxFaceRecognizeInfo> infos)
{
    static bool isInDetect = false;
    if (isInDetect)
    {
        return;
    }
    isInDetect = true;

    updateAvatarInfo(img, detectedFaceInfos);
    if (infos.count() > 0) {
        NxFaceRecognizeInfo bestInfo;
        for (const auto& info : infos)
        {
            if (info.score > kFaceDetectThreshold)
            {
                bestInfo = info;
            }
        }

        const auto recognizedInfo = userService->getUserIdentityInfo(bestInfo.identityUid);
        if (recognizedInfo.getQUid() == currentIdentity.getQUid())
        {
            ui->identityInfo->setText(currentLoginName + tr(" has record face info"));
            ui->capture->setEnabled(true);
        }
        else
        {
            if (recognizedInfo.getRoleType() == Role_Patient)
            {
                const auto patient = userService->getPatientByIdentity(recognizedInfo.getQUid());
                ui->identityInfo->setText(QString::fromStdString(patient.PatientName) + tr(" has record face info"));
            }
            else
            {
                ui->identityInfo->setText(QString::fromStdString(recognizedInfo.LoginName) + tr(" has record face info"));
            }
            ui->capture->setEnabled(false);
        }

        identityTimer.restart();
    }
    isInDetect = false;
}

void CUserInfo::slotCapture()
{
    if (recordStatus == RS_CAPTURE) {
        recordStatus = RS_RECAPTURE;
        stopDetect();
        ui->capture->setText(tr("ReCapture"));
        ui->capture->setEnabled(true);
        ui->confirm->setEnabled(true);
    }
    else {
        recordStatus = RS_CAPTURE;
        startDetect();
        ui->capture->setText(tr("Capture"));
        ui->capture->setEnabled(false);
        ui->confirm->setEnabled(false);
    }
}

void CUserInfo::slotConfirm()
{
    QByteArray ba;
    QBuffer imgBuffer(&ba);
    imgBuffer.open(QIODevice::WriteOnly);
    capturedImg.save(&imgBuffer, "jpg");

    currentIdentity.Avatar = std::vector(ba.begin(), ba.end());
    const auto feature = CUserService::extractFeature(capturedImg);
    currentIdentity.Feature.resize(1024 * sizeof(float));
    memcpy(currentIdentity.Feature.data(), feature.data(), 1024 * sizeof(float));
    ba.clear();
    showAvatar();

    userService->stopFaceDetector();
    ui->stackedWidget->setCurrentWidget(ui->info);
    releaseDetect();
}

void CUserInfo::slotSwitchToRecordFace()
{
    recordStatus = RS_CAPTURE;
    ui->capture->setText(tr("Capture"));
    ui->capture->setEnabled(false);
    ui->confirm->setEnabled(false);

    userService->initFaceDetector();

    connect(userService, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), Qt::QueuedConnection);


    if (!userService->isCameraOpened()) {
        ui->identityInfo->setText(tr("Not find camera!"));
    }
    else
    {
        userService->startFaceDetector();
    }
    ui->stackedWidget->setCurrentWidget(ui->recordFace);
}

bool CUserInfo::checkInfo() const
{
    QRegularExpression passwordRegex("^(?=.*[a-zA-Z])(?=.*[!@#$%^&*()\\[\\]{}<>?+\\d]).{8,20}$");
    QRegularExpressionMatch match = passwordRegex.match(ui->PasswordLine->text());
    if(ui->UserName->text().isEmpty())
    {
        ui->usernameStatus->setVisible(true);
        ui->usernamePrompt->setVisible(true);
        ui->usernamePrompt->setText(tr("The user name is empty"));
        return false;
    }
    if (ui->Name->text().isEmpty())
    {
        return false;
    }
    if (ui->PasswordLine->text().isEmpty())
    {
        ui->passwordStatus->setVisible(true);
        ui->passwordPrompt->setVisible(true);
        ui->passwordPrompt->setText(tr("Please input password"));
        return false;
    }
    if (!match.hasMatch())
    {
        ui->passwordStatus->setVisible(true);
        ui->passwordPrompt->setText(tr("Password format error"));
        ui->passwordPrompt->setVisible(true);
        return false;
    }
    if (!userService->getUserByLoginName(ui->UserName->text()).empty()&&ui->UserName->isEnabled())
    {

        ui->usernameStatus->setVisible(true);
        ui->usernamePrompt->setVisible(true);
        ui->usernamePrompt->setText(tr("The user name already exists"));
        return false;

    }

    return true;

}

void CUserInfo::updateAvatarInfo(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos)
{
    //LOG_INFO("Recognize Face.");
    constexpr auto borderWidth = 3;
    const auto widgetWidth = ui->video->width() - borderWidth * 2;
    const auto widgetHeight = ui->video->height() - borderWidth * 2;
    const auto displayRegion = QRect(img.width() / 2 - widgetWidth / 2, img.height() / 2 - widgetHeight / 2, widgetWidth, widgetHeight);
    //capturedImg = img.copy(displayRegion);
    QString identityTip = "";
    if (!detectedFaceInfos.empty()) {
        NxFaceDetectedInfo bestInfo;
        QImage faceImg;

        bestInfo.score = 0;
        for (const auto& info : detectedFaceInfos) {
            const auto areaRatio = (img.width() * img.height()) / (info.width * info.height);
            if (info.score > bestInfo.score && areaRatio < 68) { // 4500 为经验值, 用于判断人脸大小，太小则人脸较远
                bestInfo = info;
            }
        }
        if (bestInfo.score > kFaceDetectThreshold) {

            ui->capture->setEnabled(true);
            identityTip = tr("Face is detected");
            const auto centerDistance = sqrt(pow(bestInfo.x + bestInfo.width / 2 - faceArea.center().x(), 2)
                + pow(bestInfo.y + bestInfo.height / 2 - faceArea.center().y(), 2));
            // abs(area - preArea) > 250 ||
            if (centerDistance > 90) { // 当头像相对上一次距离变化大于一定值才更新窗口
                faceArea = QRect(bestInfo.x, bestInfo.y, bestInfo.width, bestInfo.height);
            }
            auto cropArea = QRect(faceArea.center().x() - faceArea.height(), faceArea.center().y() - faceArea.height() - 20, faceArea.height() * 2, faceArea.height() * 2);
            CRecordFace::adjustCropArea(cropArea, img.width(), img.height());
            faceImg = img.copy(cropArea);
            const auto showImg = faceImg.scaled(widgetWidth, widgetHeight, Qt::KeepAspectRatioByExpanding);
            auto w = showImg.width();
            auto h = showImg.height();
            capturedImg = showImg.copy();
            const auto pixmap = CRecordFace::pixmapToRound(QPixmap::fromImage(showImg), ui->video->width() / 2 - borderWidth);
            ui->video->setPixmap(pixmap);
        }
        else {
            identityTip = tr("The image of the face is not clear, please come closer");
            ui->capture->setEnabled(false);
            faceArea = QRect(0, 0, 0, 0);
            auto cropArea = QRect(bestInfo.x + bestInfo.width / 2 - bestInfo.height,
                bestInfo.y - bestInfo.height / 2 - 20,
                bestInfo.height * 2,
                bestInfo.height * 2);
            CRecordFace::adjustCropArea(cropArea, img.width(), img.height());
            faceImg = img.copy(cropArea);
            const auto showImg = faceImg.scaled(widgetWidth, widgetHeight, Qt::KeepAspectRatioByExpanding);
            auto w = showImg.width();
            auto h = showImg.height();
            capturedImg = showImg.copy();
            const auto pixmap = CRecordFace::pixmapToRound(QPixmap::fromImage(showImg), ui->video->width() / 2 - borderWidth);
            ui->video->setPixmap(pixmap);
        }
    }
    else { // 未检测到人脸，则显示原始图像
        ui->identityInfo->setText("");
        const auto showImg = img.copy(displayRegion);
        const auto pixmap = CRecordFace::pixmapToRound(QPixmap::fromImage(showImg), ui->video->width() / 2 - borderWidth);
        ui->video->setPixmap(pixmap);
        ui->capture->setEnabled(false);
        faceArea = QRect(0, 0, 0, 0);
    }

    if (identityTimer.elapsed() > 500) {
        ui->identityInfo->setText(identityTip);
    }
}

void CUserInfo::releaseDetect() const
{
    disconnect(userService, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));
    userService->releaseFaceDetector();
}

void CUserInfo::stopDetect() const
{
    connect(userService, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));
    userService->stopFaceDetector();
}

void CUserInfo::startDetect()
{
    userService->startFaceDetector();
    if (!userService->isCameraOpened()) {
        ui->identityInfo->setText(tr("Not find camera!"));
    }
    else {
        connect(userService, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>)));
        connect(userService, SIGNAL(signalBroadcastFRInfos(QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFRInfos(QVector<NxFaceRecognizeInfo>)));
    }
    identityTimer.start();
}

void CUserInfo::showAvatar() const
{
    if (!currentIdentity.Avatar.empty()) {
        QByteArray ba;
        for (const char i : currentIdentity.Avatar)
        {
            ba.append(i);
        }

        QImage img;
        img.loadFromData(ba, "jpg");
        ba.clear();
        auto showImg = QPixmap::fromImage(img.scaled(ui->showFaceLabel->width(), ui->showFaceLabel->height(), Qt::KeepAspectRatio));

        const auto radius = ui->showFaceLabel->width() / 2;
        const QSize size(ui->showFaceLabel->width(), ui->showFaceLabel->height());
        QBitmap mask(size);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
        painter.setBrush(QColor(220, 0, 0));
        painter.drawEllipse(QPoint(radius, radius), radius, radius);
        showImg.setMask(mask);
        ui->showFaceLabel->setPixmap(showImg);
    }
}

void CUserInfo::slotCloseWindow()
{
    ui->UserName->clear();
    ui->PasswordLine->clear();
    ui->Name->clear();
    ui->UserType->setCurrentIndex(0);
    ui->Departments->clear();

    ui->passwordStatus->setVisible(false);
    ui->passwordPrompt->setVisible(false);
    ui->usernameStatus->setVisible(false);
    ui->usernamePrompt->setVisible(false);
    releaseDetect();
    emit signalUserInfoClose();
}
END_NX_NAMESPACE