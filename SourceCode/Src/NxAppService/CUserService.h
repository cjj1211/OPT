/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __USER_SERVICE_H
#define __USER_SERVICE_H
#include "nxappservice_global.h"
#include "IAppService.h"

#include <QImage>
#include "NxFaceIdentifier/NxFaceIdenfierGlobal.h"

BEGIN_NX_NAMESPACE
class CPatientRepository;
class CFaceIdentifier;
class CFeatureDetector;
struct User;
struct Patient;
struct IdentityInfo;
enum GenderEmu;
class CUserRepository;
class CIdentifyInfoRepository;

class NX_APP_SERVICE_EXPORT CUserService
    :public IAppService
{
    Q_OBJECT
public:
    CUserService(QObject* parent = nullptr);
    
    virtual ~CUserService() override;

    std::vector<IdentityInfo> getUserByLoginName(const QString& loginname);
    std::vector<User> getUserByIdentifyFK(const QString& IdentifyFK);
    void UpdateIdentityInfo(IdentityInfo info);
    void UpdateUser(User userInfo);

    void CreateIdentityInfo(IdentityInfo info);
    void CreateUser(User userInfo);
    std::vector<User> getAllUser();
    IdentityInfo getUserIdentityInfo(const QString& infoUid) const;
    Patient getPatientByIdentity(const QString& identityFk) const;
    void deleteUser(const  QString& infoUid, const  QString& userUid) const;

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