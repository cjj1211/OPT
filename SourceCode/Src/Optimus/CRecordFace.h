/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   November 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_RECORD_FACE_H
#define __C_RECORD_FACE_H
#include "Global.h"
#include "CDragableDialog.h"
#include "NxFaceIdentifier/CFeatureDetector.h"
#include "NxFaceIdentifier/CFaceIdentifier.h"
#include "ui_CRecordFace.h"
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class CRecordFace; };
QT_END_NAMESPACE

BEGIN_NX_NAMESPACE

class CPatientInfoService;
class CRecordFace : public CDragableDialog
{
    Q_OBJECT
        QT_TRANSLATE_HANDLER
        enum RecordStatus {
        RS_CAPTURE, // 正在捕获图像
        RS_RECAPTURE // 已捕获，点击重拍
    };
public:
    CRecordFace(CPatientInfoService* paService, const QString& patientUid, QWidget* parent = nullptr);
    ~CRecordFace() override;

    static QPixmap pixmapToRound(const QPixmap& src, int radius);
    static void adjustCropArea(QRect& faceArea, const int imgW, const int imgH);

protected:
    void closeEvent(QCloseEvent* e) override;
    void hideEvent(QHideEvent* e) override;
    void showEvent(QShowEvent* e) override;

private:
    void stopDetect() const;
    void startDetect();

    static QPixmap createRoundMask(int radius);

public slots:
    void slotBroadcastFrame(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos, const QVector<NxFaceRecognizeInfo>
        & infos);
    void slotCapture();
    void slotConfirm();

signals:
    void signalUpdateAvator(QImage img);

private:
    void updateAvatarInfo(QImage img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos);


private:
    Ui::CRecordFace* ui;
    CPatientInfoService* service;
    QString curPatientUid;
    QImage capturedImg;
    RecordStatus recordStatus;
    QRect faceArea; // 人脸尺寸
    QElapsedTimer identityTimer; // 识别到当前用户的计时器

};

END_NX_NAMESPACE
#endif