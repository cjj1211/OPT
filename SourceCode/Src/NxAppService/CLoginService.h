/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef _C_LOGIN_SERVICE_H_
#define _C_LOGIN_SERVICE_H_

#include "nxappservice_global.h"
#include "IAppService.h"
#include "NxFaceIdentifier/NxFaceIdenfierGlobal.h"
#include <QImage>

#include "NxEntity/Patient.h"

namespace NX
{
    class CPatientRepository;
}

BEGIN_NX_NAMESPACE
    class CUserRepository;
class CIdentifyInfoRepository;
struct User;
struct IdentityInfo;
class CFeatureDetector;
class CFaceIdentifier;

class NX_APP_SERVICE_EXPORT CLoginService :public IAppService
{
    Q_OBJECT
public:
    CLoginService(QObject* parent = nullptr);
    ~CLoginService() override;

    IdentityInfo getIdentityByUid(const QString& uid) const;
    std::vector<IdentityInfo> getIdentityByLoginName(const QString& loginName) const;
    std::vector<User> getUserByIdentifyFK(const QString& IdentifyFK) const;
    Patient getPatientByIdentifyFK(const QString& IdentifyFK) const;
    void UpdateIdentityInfo(const IdentityInfo& info) const;

    // Face detect
    void initFaceDetector(const QString& loginName = "");

    static std::vector<float> extractFeature(const QImage& img);
    bool isCameraOpened() const;
    void startFaceDetector() const;
    void stopFaceDetector() const;
    void releaseFaceDetector();

signals:
    void signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>);

private:
    std::unique_ptr<CUserRepository> userRepository;
    std::unique_ptr<CIdentifyInfoRepository> identityInfoRepository;
    std::unique_ptr<CPatientRepository> patientRepository;

    QVector<FaceInfo> faceInfos;
    std::vector<float> faceFeature;
    CFeatureDetector* faceDetector;
    CFaceIdentifier* faceIdentifier;
};

END_NX_NAMESPACE
#endif

