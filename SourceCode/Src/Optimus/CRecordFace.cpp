#include "CRecordFace.h"
#include "NxEntity/Patient.h"
#include "NxEntity/IdentityInfo.h"
#include "NxAppService/CPatientInfoService.h"
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QPainter>
#include <QBitmap>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>


BEGIN_NX_NAMESPACE
constexpr float kFaceDetectThreshold = 0.98f;

CRecordFace::CRecordFace(CPatientInfoService* paService, const QString& patientUid, QWidget* parent)
    : CDragableDialog(parent)
    , ui(new Ui::CRecordFace())
    , service(paService)
    , curPatientUid(patientUid)
    , recordStatus(RS_CAPTURE)
{
    ui->setupUi(this);
    ui->defaultFemaleContainer->hide();
    ui->defaultMaleContainer->hide();
    ui->capture->setEnabled(false);
    ui->confirm->setEnabled(false);

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

    service->initFaceDetector();
    if (!service->isCameraOpened()) {
        ui->identityInfo->setText(tr("Not find camera!"));
    }
    else {
        connect(service, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));
    }
    connect(ui->capture, SIGNAL(clicked()), this, SLOT(slotCapture()));
    connect(ui->confirm, SIGNAL(clicked()), this, SLOT(slotConfirm()));
}

CRecordFace::~CRecordFace()
{
    delete ui;
}

void CRecordFace::closeEvent(QCloseEvent* e) {
    stopDetect();
}

void CRecordFace::hideEvent(QHideEvent* e) {
    stopDetect();
}

void CRecordFace::showEvent(QShowEvent* e) {
    startDetect();
}

void CRecordFace::stopDetect() const
{
    disconnect(service, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));
    service->stopFaceDetector();
}

void CRecordFace::startDetect() {
    service->startFaceDetector();
    if (!service->isCameraOpened()) {
        ui->identityInfo->setText(tr("Not find camera!"));
    }
    else {
        connect(service, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SLOT(slotBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));
    }
    identityTimer.start();
}

QPixmap CRecordFace::createRoundMask(const int radius)
{
    cv::Mat img(radius * 2, radius * 2, CV_8UC4);
    cv::rectangle(img, cv::Rect(0, 0, radius * 2, radius * 2), { 0,0,0,255 }, -1, cv::LINE_AA);
    cv::circle(img, cv::Point(radius, radius), radius, { 0,0,0,0 }, -1, cv::LINE_AA);

    const auto pSrc = static_cast<const uchar*>(img.data);
    const QImage image(pSrc, img.cols, img.rows, static_cast<int>(img.step), QImage::Format_ARGB32);

    return QPixmap::fromImage(image.copy());
}

QPixmap CRecordFace::pixmapToRound(const QPixmap& src, int radius)
{
    if (src.isNull()) {
        return {};
    }
    const QSize size(2 * radius, 2 * radius);

    QImage resultImage(size, QImage::Format_ARGB32_Premultiplied);
    const QPixmap head_mask = createRoundMask(radius);
    QPainter painter(&resultImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(0, 0, head_mask);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOut);
    painter.drawPixmap(0, 0, src.scaled(size));
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.end();

    return QPixmap::fromImage(resultImage);
}

void CRecordFace::updateAvatarInfo(QImage img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos)
{
    //LOG_INFO("Recognize Face.");
    constexpr auto borderWidth = 3;
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
        if (bestInfo.score > kFaceDetectThreshold) { // 识别到清晰的人脸

            ui->capture->setEnabled(true);
            identityTip = tr("Face is detected");

            const auto centerDistance = sqrt(pow(bestInfo.x + bestInfo.width / 2 - faceArea.center().x(), 2)
                + pow(bestInfo.y + bestInfo.height / 2 - faceArea.center().y(), 2));

            if (centerDistance > 90) { // 当头像相对上一次距离变化大于一定值才更新窗口
                faceArea = QRect(bestInfo.x, bestInfo.y, bestInfo.width, bestInfo.height);
            }
            auto cropArea = QRect(faceArea.center().x() - faceArea.height(), faceArea.center().y() - faceArea.height() - 20, faceArea.height() * 2, faceArea.height() * 2);
            adjustCropArea(cropArea, img.width(), img.height());
            faceImg = img.copy(cropArea);

            const auto showImg = faceImg.scaled(widgetWidth, widgetHeight, Qt::KeepAspectRatioByExpanding);

            capturedImg = showImg.copy();
            const auto pixmap = pixmapToRound(QPixmap::fromImage(showImg), ui->video->width() / 2 - borderWidth);
            ui->video->setPixmap(pixmap);
        }
        else { // 人脸不清晰
            identityTip = tr("The image of the face is not clear, please come closer");
            ui->capture->setEnabled(false);
            faceArea = QRect(0, 0, 0, 0);
            auto cropArea = QRect(bestInfo.x + bestInfo.width / 2 - bestInfo.height,
                bestInfo.y - bestInfo.height / 2 - 20,
                bestInfo.height * 2,
                bestInfo.height * 2);
            adjustCropArea(cropArea, img.width(), img.height());
            faceImg = img.copy(cropArea);
            const auto showImg = faceImg.scaled(widgetWidth, widgetHeight, Qt::KeepAspectRatioByExpanding);
            auto w = showImg.width();
            auto h = showImg.height();
            capturedImg = showImg.copy();
            const auto pixmap = pixmapToRound(QPixmap::fromImage(showImg), ui->video->width() / 2 - borderWidth);
            ui->video->setPixmap(pixmap);
        }
    }
    else { // 未检测到人脸，则显示原始图像
        ui->identityInfo->setText("");
        const auto showImg = img.copy(displayRegion);
        const auto pixmap = pixmapToRound(QPixmap::fromImage(showImg), ui->video->width() / 2 - borderWidth);
        ui->video->setPixmap(pixmap);
        ui->capture->setEnabled(false);
        faceArea = QRect(0, 0, 0, 0);
    }

    if (identityTimer.elapsed() > 500) {
        ui->identityInfo->setText(identityTip);
    }
}

void CRecordFace::adjustCropArea(QRect& faceArea, const int imgW, const int imgH)
{
    int preW = faceArea.width();
    int preH = faceArea.height();
    if (faceArea.left() < 0)
    {
        faceArea.setX(0);
    }

    if (faceArea.right() > imgW)
    {
        faceArea.setX(imgW - faceArea.width());
    }

    if (faceArea.top() < 0)
    {
        faceArea.setY(0);
    }

    if (faceArea.bottom() > imgH)
    {
        faceArea.setY(imgH - faceArea.height());
    }

    faceArea.setWidth(preW);
    faceArea.setHeight(preH);
}

// 不提取人脸，使用中心截图显示
//void CRecordFace::slotBroadcastFrame(QImage img, QVector<NxFaceDetectedInfo> detectedFaceInfos) {
//
//    //LOG_INFO("Recognize Face.");
//    auto borderWidth = 3;
//    auto widgetWidth = ui->video->width() - borderWidth * 2;
//    auto widgetHeight = ui->video->height() - borderWidth * 2;
//    auto displayRegion = QRect(img.width() / 2 - widgetWidth / 2, img.height() / 2 - widgetHeight / 2, widgetWidth, widgetHeight);
//    capturedImg = img.copy(displayRegion);
//
//    if (detectedFaceInfos.size() > 0) {
//        NxFaceDetectedInfo bestInfo;
//        bestInfo.score = 0;
//        bool hasGoodFace = false;
//        for (auto& info : detectedFaceInfos) {
//            if (info.x > displayRegion.x() &&
//                info.y > displayRegion.y() &&
//                info.x + info.width < displayRegion.right() &&
//                info.y + info.height < displayRegion.bottom()) { // 判断识别到的人脸在当前显示区域内
//                if (info.score > bestInfo.score) {
//                    bestInfo = info;
//                }
//                hasGoodFace = true;
//            }
//        }
//        QPainter painter(&img);
//        painter.setPen(Qt::gray);//brush画刷填充，setPen可以画圆圈
//        painter.drawRect(bestInfo.x, bestInfo.y, bestInfo.width, bestInfo.height);//在image中(0,0)为左上角，宽20、高20的矩形内画圆
//        if (hasGoodFace) {
//            if (bestInfo.score > kFaceDetectThreashold) {
//                ui->identityInfo->setText(tr("Face is detected") + QString::number(bestInfo.score, 'f', 2));
//                ui->capture->setEnabled(true);
//            }
//            else {
//                ui->identityInfo->setText(tr("The image of the face is not clear, please come closer"));
//                ui->capture->setEnabled(false);
//            }
//        }
//        else {
//            ui->identityInfo->setText("");
//            ui->capture->setEnabled(false);
//        }
//    }
//    else {
//        ui->identityInfo->setText("");
//        ui->capture->setEnabled(false);
//    }
//
//    auto showImg = img.copy(img.width() / 2 - widgetWidth / 2, img.height() / 2 - widgetHeight / 2, widgetWidth, widgetHeight);
//    auto pixmap = pixmapToRound(QPixmap::fromImage(showImg), ui->video->width() / 2 - borderWidth);
//    ui->video->setPixmap(pixmap);
//}

void CRecordFace::slotBroadcastFrame(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos,
    const QVector<NxFaceRecognizeInfo>& infos) {
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
        if (!curPatientUid.isEmpty())
        {
            Patient patientInfo;
            IdentityInfo identityInfo;
            service->getPatientByUid(curPatientUid, patientInfo);
            service->getPatientIdentifyInfo(QString::fromStdString(patientInfo.IdentifyFK), identityInfo);
            if (bestInfo.identityUid == identityInfo.getQUid()) // 已识别到的人脸中包含当前患者的人脸
            {
                ui->identityInfo->setText(QString::fromStdString(patientInfo.PatientName) + tr(" has record face info"));
                ui->capture->setEnabled(true);
            }
            else
            {
                ui->identityInfo->setText(tr("This face has been record by other people."));
                ui->capture->setEnabled(false);
                qApp->processEvents();
            }
        }

        identityTimer.restart();
    }
    else
    {
        ui->capture->setEnabled(true);
    }
    isInDetect = false;
}

void CRecordFace::slotCapture()
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

void CRecordFace::slotConfirm()
{
    emit signalUpdateAvator(capturedImg);
    close();
}
END_NX_NAMESPACE

