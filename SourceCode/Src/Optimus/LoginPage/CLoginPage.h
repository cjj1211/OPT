#ifndef CLOGINPAGE_H
#define CLOGINPAGE_H

#include <QElapsedTimer>

#include "../Global.h"
#include "ui_CLoginPage.h"
#include "NxEntity/User.h"
#include "NxEntity/IdentityInfo.h"
#include "NxFaceIdentifier/NxFaceIdenfierGlobal.h"

BEGIN_NX_NAMESPACE

class CLoginService;
struct IdentityInfo;
struct User;
class CLoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit CLoginPage(QWidget *parent = nullptr);
    ~CLoginPage() override;

    QString getCurrentUserName() const;

protected:
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;

public slots:
    void  slotShutdown();
   
signals:
    void signalLogin(bool isLog);
    void signalFirstLogin();
    void signalShutdown();
    void signalAdminLogin();

public slots:
    void slotLogin();
    void slotBroadcastFrame(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos, QVector<NxFaceRecognizeInfo> infos);
    static QPixmap createRoundMask(int radius);

private:
    void updateAvatar(const QImage& img, const QVector<NxFaceDetectedInfo>& detectedFaceInfos);
    std::unique_ptr<Ui::CLoginPage> ui;
    CLoginService* loginService;
    IdentityInfo currentIn;
    User currentUser;
    QString currentUserName;

    QImage capturedImg;
    QRect faceArea; // 人脸尺寸
    QElapsedTimer identityTimer; // 识别到当前用户的计时器
    QString identifiedID{""};
    bool isFaceIdentified{ false };
};

#endif // CLOGINPAGE_H
END_NX_NAMESPACE