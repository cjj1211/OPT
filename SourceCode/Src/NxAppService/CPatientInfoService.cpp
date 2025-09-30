#include "CPatientInfoService.h"

#include "NxDBManager/CIdentifyInfoRepository.h"
#include "NxDBManager/CPatientRepository.h"
#include "NxFaceIdentifier/CFaceIdentifier.h"
#include "NxFaceIdentifier/CFeatureDetector.h"

BEGIN_NX_NAMESPACE

CPatientInfoService::CPatientInfoService(QObject* parent)
	:IAppService(parent)
	, patientRepository(new CPatientRepository())
	, identityInfoRepository(new CIdentifyInfoRepository())
	, faceDetector(nullptr)
	, faceIdentifier(nullptr)
{
}

CPatientInfoService::~CPatientInfoService()
{
}

bool CPatientInfoService::getPatientByUid(const QString& uid, Patient& patient) const
{
    return patientRepository->GetByUID(uid, patient);
}

bool CPatientInfoService::getPatientIdentifyInfo(const QString& uid, IdentityInfo& identityInfo) const
{
    return identityInfoRepository->GetByUID(uid, identityInfo);
}

bool CPatientInfoService::isCaseIdDuplicate(const QString& caseId, const QString& uid) const
{
    return patientRepository->isCaseIdDuplicate(caseId, uid);
}

bool CPatientInfoService::deletePatientByUid(const QString& patientUid) const
{
    return  patientRepository->deleteWithRefInfoByUid(patientUid);
}

bool CPatientInfoService::updatePatient(const IdentityInfo& identityInfo, const Patient& patient) const
{
    return patientRepository->updatePatient(identityInfo, patient);
}

bool CPatientInfoService::createPatient(const IdentityInfo& identityInfo, const Patient& patient) const
{
    return patientRepository->createPatient(identityInfo, patient);
}

std::vector<float> CPatientInfoService::extractFeature(const QImage& img)
{
    CFeatureDetector detector;
    std::vector<float> feature(1024);
    feature.clear();
    detector.ExtractFeature(img, feature);
    return feature;
}

void CPatientInfoService::initFaceDetector()
{
    faceDetector = std::make_unique<CFeatureDetector>();
    const auto identifyInfos = identityInfoRepository->getAllIdInfos();
    for (auto identityInfo : identifyInfos)
    {
        if (!identityInfo.Feature.empty()) {
            faceFeature.resize(1024);
            memcpy(faceFeature.data(), identityInfo.Feature.data(), 1024 * sizeof(float));
            faceInfos.push_back({ 1, 100, 100, identityInfo.getQUid(), faceFeature });
        }
    }

    faceIdentifier = std::make_unique<CFaceIdentifier>(faceInfos);

    // 设置人脸识别相似度阈值
    faceIdentifier->SetIdentifyThreshold(0.75);
    // 开启活体检测功能
    faceIdentifier->EnableAntiSproof(false);
    // 开启绘制检测到的人脸功能
    faceIdentifier->EnableDrawDetectedFaces(false);
    // 开启绘制人脸信息功能
    faceIdentifier->EnableDrawName(false);
    // 开启绘制相似度功能
    faceIdentifier->EnableDrawSimilarity(false);
    // 开启绘制已识别人脸功能
    faceIdentifier->EnableDrawRecognizedFaces(false);

    // 建立信号槽连接，获取采集到的图像
    connect(faceIdentifier.get(), SIGNAL(SignalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));

    faceIdentifier->StartIdentify();
}

bool CPatientInfoService::isCameraOpened() const
{
    if (nullptr == faceIdentifier) {
        return false;
    }
    return faceIdentifier->IsCameraOpen();
}

void CPatientInfoService::startFaceDetector() const
{
    if (nullptr == faceIdentifier) return;
    faceIdentifier->StartIdentify();
}

void CPatientInfoService::stopFaceDetector() const
{
    if (nullptr == faceIdentifier) return;
    faceIdentifier->FinishIdentify();

    //faceInfos.clear();
    //delete faceDetector;
    //faceDetector = nullptr;

    //delete faceIdentifier;
    //faceIdentifier = nullptr;
}

END_NX_NAMESPACE
