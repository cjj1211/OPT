#ifndef C_PATIENT_INFO_SERVICE
#define C_PATIENT_INFO_SERVICE
#include "IAppService.h"
#include "NxFaceIdentifier/NxFaceIdenfierGlobal.h"
#include <QImage>

BEGIN_NX_NAMESPACE

struct IdentityInfo;
struct Patient;

class CPatientRepository;
class CIdentifyInfoRepository;
class CFeatureDetector;
class CFaceIdentifier;

class NX_APP_SERVICE_EXPORT CPatientInfoService :public IAppService
{
    Q_OBJECT
public:
    CPatientInfoService(QObject* parent = nullptr);

    virtual ~CPatientInfoService() override;

    bool getPatientByUid(const QString& uid, Patient& patient) const;

    bool getPatientIdentifyInfo(const QString& uid, IdentityInfo& identityInfo) const;

    bool isCaseIdDuplicate(const QString& caseId, const QString& uid) const;

    bool deletePatientByUid(const QString& patientUid) const;

    bool updatePatient(const IdentityInfo& identityInfo, const Patient& patient) const;

    bool createPatient(const IdentityInfo& identityInfo, const Patient& patient) const;

    static std::vector<float> extractFeature(const QImage& img);

    void initFaceDetector();

    bool isCameraOpened() const;

    void startFaceDetector() const;

    void stopFaceDetector() const;

signals:
    void signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>);

private:
    std::unique_ptr<CPatientRepository> patientRepository;
    std::unique_ptr<CIdentifyInfoRepository> identityInfoRepository;
    QVector<FaceInfo> faceInfos;
    std::vector<float> faceFeature;
    std::unique_ptr<CFeatureDetector> faceDetector;
    std::unique_ptr<CFaceIdentifier> faceIdentifier;
};

END_NX_NAMESPACE
#endif