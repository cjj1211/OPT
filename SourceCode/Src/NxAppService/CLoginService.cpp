#include "CLoginService.h"

#include "NxDBManager/CIdentifyInfoRepository.h"
#include "NxDBManager/CPatientRepository.h"
#include "NxDBManager/CUserRepository.h"
#include "NxFaceIdentifier/CFaceIdentifier.h"
#include "NxFaceIdentifier/CFeatureDetector.h"

BEGIN_NX_NAMESPACE

CLoginService::CLoginService(QObject* parent)
    :IAppService(parent)
    , userRepository(new CUserRepository())
    , identityInfoRepository(new CIdentifyInfoRepository())
    , patientRepository(new CPatientRepository())
    , faceDetector(nullptr)
    , faceIdentifier(nullptr)
{
}

CLoginService::~CLoginService()
{
    delete faceDetector;
    delete faceIdentifier;
}

IdentityInfo CLoginService::getIdentityByUid(const QString& uid) const
{
    return identityInfoRepository->getIdentityInfoByUid(uid);
};

std::vector<IdentityInfo> CLoginService::getIdentityByLoginName(const QString& loginName) const
{
    auto identityInfo = identityInfoRepository->getIdInfoByLoginName(loginName);
    return identityInfo;
}

std::vector<User> CLoginService::getUserByIdentifyFK(const QString& IdentifyFK) const
{
    auto userList = userRepository->getUserByIdentifyFK(IdentifyFK);
    return userList;
}

Patient CLoginService::getPatientByIdentifyFK(const QString& IdentifyFK) const
{
    return patientRepository->getPatientByIdentifyFK(IdentifyFK);
}

void CLoginService::UpdateIdentityInfo(const IdentityInfo& info) const
{
    identityInfoRepository->Update(info);
}

void CLoginService::initFaceDetector(const QString& loginName)
{
	if (faceDetector)
	{
		return;
	}

	faceDetector = new CFeatureDetector();
	const auto identityInfos = identityInfoRepository->getAllIdInfos();
	for(auto identityInfo : identityInfos)
	{
		if (!identityInfo.Feature.empty()) {
            const auto displayName = identityInfo.getQUid();

			faceFeature.resize(1024);
			memcpy(faceFeature.data(), identityInfo.Feature.data(), 1024 * sizeof(float));
			faceInfos.push_back({ 1, 100, 100, displayName, faceFeature });
		}
	}

	faceIdentifier = new CFaceIdentifier(faceInfos);

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
	connect(faceIdentifier, SIGNAL(SignalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));
}

std::vector<float> CLoginService::extractFeature(const QImage& img)
{
	CFeatureDetector detector;
	std::vector<float> feature(1024);
	feature.clear();
	detector.ExtractFeature(img, feature);
	return feature;
}

bool CLoginService::isCameraOpened() const
{
	if (nullptr == faceIdentifier) {
		return false;
	}
	return faceIdentifier->IsCameraOpen();
}

void CLoginService::startFaceDetector() const
{
	if (nullptr == faceIdentifier) return;
	faceIdentifier->StartIdentify();
}

void CLoginService::stopFaceDetector() const
{
	if (nullptr == faceIdentifier) return;
	faceIdentifier->FinishIdentify();
}

void CLoginService::releaseFaceDetector()
{
	if (nullptr == faceIdentifier) return;
	// 释放信号槽连接，获取采集到的图像
	disconnect(faceIdentifier, SIGNAL(SignalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>)), this, SIGNAL(signalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>)));
	// 释放信号槽连接，获取识别到的人脸信息
	disconnect(faceIdentifier, SIGNAL(SignalBroadcastFRInfos(QVector<NxFaceRecognizeInfo>)), this, SIGNAL(signalBroadcastFRInfos(QVector<NxFaceRecognizeInfo>)));

	faceIdentifier->FinishIdentify();
	faceIdentifier->deleteLater();
	faceIdentifier = nullptr;
	delete faceDetector;
	faceDetector = nullptr;
}

END_NX_NAMESPACE