#ifndef CUSERINFO_H
#define CUSERINFO_H

#include <QElapsedTimer>

#include "../Global.h"
#include "ui_CUserInfo.h"
#include "NxEntity/User.h"
#include "NxEntity/IdentityInfo.h"
#include "NxFaceIdentifier/CFeatureDetector.h"
#include "NxFaceIdentifier/CFaceIdentifier.h"
#include "Optimus/CDragableDialog.h"

BEGIN_NX_NAMESPACE
    class CUserService;
struct IdentityInfo;
struct User;

class CUserInfo : public CDragableDialog
{
    Q_OBJECT

    enum RecordStatus {
        RS_CAPTURE, // 正在捕获图像
        RS_RECAPTURE // 已捕获，点击重拍
    };
public:
    explicit CUserInfo(CUserService* service_, QString loginName_ ,QWidget *parent = nullptr);
    ~CUserInfo() override;
    void initWidget(const QString& loginName );
    bool isCreate = true;
private slots:
    void slotCloseWindow();
    void slotSaveUserInfo();

public slots:
    void slotBroadcastFrame(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos, QVector<NxFaceRecognizeInfo> infos);
    void slotCapture();
    void slotConfirm();
    void slotSwitchToRecordFace() ;

signals:
    void initTableWidget();
    void signalUserInfoClose();

private:
    bool  checkInfo() const;
    void updateAvatarInfo(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos);

    void releaseDetect() const;
    void stopDetect() const;
    void startDetect();
    void showAvatar() const;

    std::unique_ptr<Ui::CUserInfo> ui;
    CUserService* userService;
    QString currentLoginName;
    IdentityInfo currentIdentity;
    User currentUser;

    RecordStatus recordStatus;
    QImage capturedImg;

    QRect faceArea; // 人脸尺寸
    QElapsedTimer identityTimer; // 识别到当前用户的计时器
};
#endif // CUSERINFO_H
END_NX_NAMESPACE