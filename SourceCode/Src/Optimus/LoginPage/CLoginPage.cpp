#include "CLoginPage.h"
#include "NxAppService/CLoginService.h"
#include "NxSystemCfg/CSystemCfg.h"
#include "Optimus/CRecordFace.h"
#include <QBitmap>
#include <QPainter>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

BEGIN_NX_NAMESPACE

constexpr float kFaceDetectThreshold = 0.98f;

CLoginPage::CLoginPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CLoginPage())
    , loginService(new CLoginService())
{
    ui->setupUi(this);
    setStyleSheet(" background-color: #000000;");
    ui->passwordPrompt->setVisible(false);
    ui->passwordStatus->setVisible(false);
    ui->loginName->setAttribute(Qt::WA_InputMethodEnabled, false);	//设置账号输入框点击时无法输入中文
    ui->password->setAttribute(Qt::WA_InputMethodEnabled, false);	//设置账号输入框点击时无法输入中文
    connect(ui->loginBtn,&QPushButton::clicked,this,&CLoginPage::slotLogin);
    connect(ui->loginName, &QLineEdit::textEdited, this, [=] {
        ui->passwordPrompt->setVisible(false);
        ui->passwordStatus->setVisible(false);
        });
    connect(ui->password, &QLineEdit::textEdited, this, [=] {
        ui->passwordPrompt->setVisible(false);
        ui->passwordStatus->setVisible(false);
        });
    connect(ui->closeBtn, &QPushButton::clicked, this, &CLoginPage::slotShutdown);
    ui->versionLabel->setText(QString::fromStdString(systemConfig.GetSystemCfg().Common.Version));

}

CLoginPage::~CLoginPage() = default;

QString CLoginPage::getCurrentUserName() const
{
    return currentUserName;
}

void CLoginPage::hideEvent(QHideEvent* event)
{
    disconnect(loginService, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));

    loginService->stopFaceDetector();
    loginService->releaseFaceDetector();

    QWidget::hideEvent(event);
}

void CLoginPage::showEvent(QShowEvent* event)
{
    connect(loginService, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));

    loginService->initFaceDetector("");
    if (!loginService->isCameraOpened()) {
        ui->identityInfo->setText(tr("Not find camera!"));
    }
    else
    {
        loginService->startFaceDetector();
    }
    QWidget::showEvent(event);
}

void CLoginPage :: slotLogin()
{
    const auto identityVec = loginService->getIdentityByLoginName(ui->loginName->text());
    if(ui->loginName->text().isEmpty())
    {
        ui->passwordPrompt->setText(tr("The user name can not be empty"));
        ui->passwordPrompt->setVisible(true);
        ui->passwordStatus->setVisible(true);
        return;
    }
    if (ui->password->text().isEmpty())
    {
        ui->passwordPrompt->setText(tr("The password can not be empty"));
        ui->passwordPrompt->setVisible(true);
        ui->passwordStatus->setVisible(true);
        return;
    }
    if (identityVec.empty())
    {
        ui->passwordPrompt->setText(tr("The user name does not exist"));
        ui->passwordPrompt->setVisible(true);
        ui->passwordStatus->setVisible(true);
    }
    else
    {
        currentIn = identityVec[0];
        if (currentIn.LoginCounts >=5)
        {
            ui->passwordPrompt->setText(tr("You must use the administrator account to unlock the account"));
            ui->passwordPrompt->setVisible(true);
            ui->passwordStatus->setVisible(true);
            currentIn.LoginCounts += 1;
            loginService->UpdateIdentityInfo(currentIn);
        }
        else
        {
            if (currentIn.Password != ui->password->text().toStdString() && !currentIn.Password.empty())
            {
                ui->passwordPrompt->setText(tr("Password entered incorrectly"));
                ui->passwordPrompt->setVisible(true);
                ui->passwordStatus->setVisible(true);
                if (currentIn.LoginName != "Sysuser")
                {
                    currentIn.LoginCounts += 1;
                }
              
                loginService->UpdateIdentityInfo(currentIn);
                return;
            }

            if (currentIn.Password == ui->password->text().toStdString() && currentIn.LoginName == "Sysuser")
            {
                const auto currentList = loginService->getUserByIdentifyFK(QString::fromStdString(currentIn.UID));
                if (currentList.empty())
                {
                    ui->passwordPrompt->setText(tr("The user name does not exist"));
                    ui->passwordPrompt->setVisible(true);
                    ui->passwordStatus->setVisible(true);
                    return;
                }
                else
                {
                    currentUser = currentList[0];
                }

                if (currentIn.isSuperAdmin())
                {
                    CLoginService::setLoginInfo(QString::fromStdString(currentIn.LoginName), QString::fromStdString(currentIn.UID), currentIn.getRoleType());
                    ui->password->clear();
                    /*if (currentIn.getRoleType() == Role_Admin)
                    {
                        emit signalAdminLogin();
                        currentUserName = QString::fromStdString(currentUser.UserName);
                    }*/
                    emit signalAdminLogin();
                    emit signalFirstLogin();
                    currentUserName = QString::fromStdString(currentUser.UserName);
                    emit signalLogin(true);

                }
                else
                {
                    CLoginService::setLoginInfo(QString::fromStdString(currentIn.LoginName), QString::fromStdString(currentIn.UID), currentIn.getRoleType());
                    ui->password->clear();
                    if (currentIn.Role == 1)
                    {
                        currentUserName = QString::fromStdString(currentUser.UserName);
                        emit signalAdminLogin();
                        emit signalLogin(true);
                    }
                   /* currentUserName = QString::fromStdString(currentUser.UserName);
                    emit signalLogin(true);*/
                }
            }
            else
            {
                const auto currentList = loginService->getUserByIdentifyFK(QString::fromStdString(currentIn.UID));
                if (currentList.empty())
                {
                    ui->passwordPrompt->setText(tr("The user name does not exist"));
                    ui->passwordPrompt->setVisible(true);
                    ui->passwordStatus->setVisible(true);
                    return;
                }
                else
                {
                    currentUser = currentList[0];
                }
                CLoginService::setLoginInfo(QString::fromStdString(currentIn.LoginName), QString::fromStdString(currentIn.UID), currentIn.getRoleType());
                ui->password->clear();
                if (currentIn.Role == 1)
                {
                    emit signalAdminLogin();
                    currentUserName = QString::fromStdString(currentUser.UserName);
                    emit signalLogin(true);
                }
                else
                {
                    currentUserName = QString::fromStdString(currentUser.UserName);
                    emit signalLogin(true);
                }
            }
        }
    }
}

void CLoginPage::slotBroadcastFrame(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos, QVector<NxFaceRecognizeInfo> infos)
{
    static bool isInDetect = false;
    if (isInDetect)
    {
        return;
    }
    isInDetect = true;
    updateAvatar(img, detectedFaceInfos);

    if (infos.count() > 0) {
        NxFaceRecognizeInfo bestInfo = infos[0];
        for (const auto& info : infos)
        {
            if (info.score > bestInfo.score)
            {
                bestInfo = info;
            }
        }

        if (identifiedID != bestInfo.identityUid)
        {
            identifiedID = bestInfo.identityUid;
            identityTimer.restart();
        }
        const auto detectIdentity = loginService->getIdentityByUid(identifiedID);
        auto displayName = QString::fromStdString(detectIdentity.LoginName);
        if (detectIdentity.getRoleType() == Role_Patient)
        {
            displayName = QString::fromStdString(loginService->getPatientByIdentifyFK(QString::fromStdString(detectIdentity.UID)).PatientName);
        }

        ui->identityInfo->setText(tr("Identified to ") + displayName + tr(", verifying..."));

        constexpr int identifyTimeThreshold = 3000; // ms
        if (identityTimer.elapsed() > identifyTimeThreshold)
        {
            // 识别到用户超过3s，才认为识别成功
            currentIn = loginService->getIdentityByUid(identifiedID);

            if (currentIn.getRoleType() == Role_Patient)
            {
                const auto patient = loginService->getPatientByIdentifyFK(QString::fromStdString(currentIn.UID));
                IAppService::setCurrentPatientUid(patient.getQUid());
                IAppService::setLoginRoleType(currentIn.getRoleType());
                emit signalLogin(true);
                loginService->stopFaceDetector();
            }
            else
            {
                ui->loginName->setText(QString::fromStdString(currentIn.LoginName));
                if (currentIn.LoginCounts >= 5)
                {
                    ui->passwordPrompt->setText(tr("You must use the administrator account to unlock the account"));
                    ui->passwordPrompt->setVisible(true);
                    ui->passwordStatus->setVisible(true);
                    currentIn.LoginCounts += 1;
                    loginService->UpdateIdentityInfo(currentIn);
                }
                else
                {
                    if (currentIn.LoginName == "Sysuser")
                    {
                        const auto currentList = loginService->getUserByIdentifyFK(QString::fromStdString(currentIn.UID))[0];
                        CLoginService::setLoginInfo(QString::fromStdString(currentIn.LoginName), QString::fromStdString(currentIn.UID), currentIn.getRoleType());
                        ui->password->clear();

                        if (currentIn.getRoleType() == Role_Admin)
                        {
                            emit signalAdminLogin();
                        }

                        if (currentIn.isSuperAdmin())
                        {
                            emit signalFirstLogin();
                        }

                        currentUserName = QString::fromStdString(currentUser.UserName);
                        emit signalLogin(true);
                        loginService->stopFaceDetector();
                    }
                    else
                    {
                        const auto currentList = loginService->getUserByIdentifyFK(QString::fromStdString(currentIn.UID));
                        if (currentList.empty())
                        {
                            ui->passwordPrompt->setText(tr("The user name does not exist"));
                            ui->passwordPrompt->setVisible(true);
                            ui->passwordStatus->setVisible(true);
                            return;
                        }
                        currentUser = currentList[0];
                        CLoginService::setLoginInfo(QString::fromStdString(currentIn.LoginName), QString::fromStdString(currentIn.UID), currentIn.getRoleType());
                        ui->password->clear();
                        if (currentIn.getRoleType() == Role_Admin)
                        {
                            emit signalAdminLogin();
                        }
                        currentUserName = QString::fromStdString(currentUser.UserName);
                        emit signalLogin(true);
                        loginService->stopFaceDetector();
                    }
                }
            }
        }
        isFaceIdentified = true;
    }
    else
    {
        identityTimer.restart();
        isFaceIdentified = false;
    }
    isInDetect = false;
}

QPixmap CLoginPage::createRoundMask(const int radius)
{
    cv::Mat img(radius * 2, radius * 2, CV_8UC4);
    cv::rectangle(img, cv::Rect(0, 0, radius * 2, radius * 2), { 0,0,0,255 }, -1, cv::LINE_AA);
    cv::circle(img, cv::Point(radius, radius), radius, { 0,0,0,0 }, -1, cv::LINE_AA);

    const auto pSrc = static_cast<const uchar*>(img.data);
    const QImage image(pSrc, img.cols, img.rows, static_cast<int>(img.step), QImage::Format_ARGB32);

    return QPixmap::fromImage(image.copy());
}

void CLoginPage::updateAvatar(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos)
{
    constexpr auto borderWidth = 9;
    const auto widgetWidth = ui->video->width() - borderWidth * 2;
    const auto widgetHeight = ui->video->height() - borderWidth * 2;
    const auto displayRegion = QRect(img.width() / 2 - widgetWidth / 2, img.height() / 2 - widgetHeight / 2, widgetWidth, widgetHeight);

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
            identityTip = tr("Unknown users are identified");
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
        faceArea = QRect(0, 0, 0, 0);
    }

    if (!isFaceIdentified) {
        ui->identityInfo->setText(identityTip);
    }
}

void CLoginPage::slotShutdown()
{
    emit signalShutdown();
}

END_NX_NAMESPACE