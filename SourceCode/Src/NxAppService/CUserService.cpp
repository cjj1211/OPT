#include "CUserService.h"
#include "NxDBManager/CUserRepository.h"
#include "NxDBManager/CIdentifyInfoRepository.h"
#include "NxDBManager/CPatientRepository.h"
#include "NxEntity/Patient.h"
#include "NxFaceIdentifier/CFaceIdentifier.h"
#include "NxFaceIdentifier/CFeatureDetector.h"

BEGIN_NX_NAMESPACE
    CUserService::CUserService(QObject* parent)
	:IAppService(parent)
	, userRepository(new CUserRepository())
	, identityInfoRepository(new CIdentifyInfoRepository())
    , faceDetector(nullptr)
    , faceIdentifier(nullptr)
{
}

CUserService::~CUserService()
{
	delete faceDetector;
	delete faceIdentifier;
}

std::vector<IdentityInfo> CUserService::getUserByLoginName(const QString& loginname)
{
	auto identityinfo = identityInfoRepository->getIdInfoByLoginName(loginname);
	return identityinfo;
}

std::vector<User> CUserService::getUserByIdentifyFK(const QString& IdentifyFK)
{

	auto userlist = userRepository->getUserByIdentifyFK(IdentifyFK);
	return userlist;
}

void CUserService::UpdateIdentityInfo(IdentityInfo info)
{
	identityInfoRepository->Update(info);
}

void CUserService::UpdateUser(User userInfo)
{
	userRepository->Update(userInfo);
}

void CUserService::CreateIdentityInfo(IdentityInfo info)
{
	identityInfoRepository->Insert(info);
}

void CUserService::CreateUser(User userInfo)
{
	userRepository->Insert(userInfo);
}

std::vector<User> CUserService::getAllUser()
{
	auto userlist = userRepository->getAllUser();
	return userlist;

}

IdentityInfo CUserService::getUserIdentityInfo(const QString& infoUid) const
{
	auto currentUserInfo = identityInfoRepository->getIdentityInfoByUid(infoUid);
	return currentUserInfo;
}

Patient CUserService::getPatientByIdentity(const QString& identityFk) const
{
	return patientRepository->getPatientByIdentifyFK(identityFk);
}

void CUserService::deleteUser(const QString& infoUid, const QString& userUid) const
{
	userRepository->DeleteByUID(userUid);
	identityInfoRepository->DeleteByUID(infoUid);
}

void CUserService::initFaceDetector(const QString& loginName)
{
	if (faceDetector)
	{
		return;
	}

	faceDetector = new CFeatureDetector();
	const auto identifyInfos = identityInfoRepository->getAllIdInfos();
	for (auto identityInfo : identifyInfos)
	{
		if (!identityInfo.Feature.empty()) {
			faceFeature.resize(1024);
			memcpy(faceFeature.data(), identityInfo.Feature.data(), 1024 * sizeof(float));
			faceInfos.push_back({ 1, 100, 100, identityInfo.getQUid(), faceFeature });
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

std::vector<float> CUserService::extractFeature(const QImage& img)
{
	CFeatureDetector detector;
	std::vector<float> feature(1024);
	feature.clear();
	detector.ExtractFeature(img, feature);
	return feature;
}

bool CUserService::isCameraOpened() const
{
	if (nullptr == faceIdentifier) {
		return false;
	}
	return faceIdentifier->IsCameraOpen();
}

void CUserService::startFaceDetector() const
{
	if (nullptr == faceIdentifier) return;
	faceIdentifier->StartIdentify();
}

void CUserService::stopFaceDetector() const
{
	if (nullptr == faceIdentifier) return;
	faceIdentifier->FinishIdentify();
}

void CUserService::releaseFaceDetector()
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


