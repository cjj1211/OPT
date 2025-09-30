#include "CIdentifyThread.h"
#include <CLogger.h>
#include <QDebug>

BEGIN_NX_NAMESPACE

CIdentifyThread::CIdentifyThread(const QVector<FaceInfo>& faceInfoVec)
	:faceInfosInDB(faceInfoVec)
{
	LOG_INFO("contructor...........................start");
	auto deviceType = SEETA_DEVICE_CPU;

	seeta::ModelSetting flSetting;
	flSetting.set_device(deviceType);
	flSetting.append("./models/face_landmarker_pts5.csta");
	faceLandmarker = std::make_unique<seeta::FaceLandmarker>(flSetting);

	seeta::ModelSetting posemodel;
	posemodel.set_device(deviceType);
	posemodel.set_id(0);
	posemodel.append("./models/pose_estimation.csta");
	poseEx = new seeta::QualityOfPoseEx(posemodel);
	poseEx->set(seeta::QualityOfPoseEx::YAW_LOW_THRESHOLD, 20);
	poseEx->set(seeta::QualityOfPoseEx::YAW_HIGH_THRESHOLD, 10);
	poseEx->set(seeta::QualityOfPoseEx::PITCH_LOW_THRESHOLD, 20);
	poseEx->set(seeta::QualityOfPoseEx::PITCH_HIGH_THRESHOLD, 10);

	faceQualityAssessor = std::make_unique<seeta::QualityAssessor>();
	faceQualityAssessor->add_rule(seeta::INTEGRITY);
	faceQualityAssessor->add_rule(seeta::RESOLUTION);
	faceQualityAssessor->add_rule(seeta::BRIGHTNESS);
	faceQualityAssessor->add_rule(seeta::CLARITY);
	faceQualityAssessor->add_rule(seeta::POSE_EX, poseEx, true);

	seeta::ModelSetting frSetting;
	frSetting.set_id(0);
	frSetting.append("./models/face_recognizer.csta");
	frSetting.set_device(deviceType);
	faceRecognizer = std::make_unique<seeta::FaceRecognizer>(frSetting);

	seeta::ModelSetting fdSetting;
	fdSetting.set_device(deviceType);
	fdSetting.append("./models/face_detector.csta");
	faceDetector = std::make_unique<seeta::FaceDetector>(fdSetting);

	seeta::ModelSetting antiSetting;
	antiSetting.set_device(deviceType);
	antiSetting.append("./models/fas_first.csta");
	antiSetting.append("./models/fas_second.csta");
	faceAntiSpoofing = std::make_unique<seeta::FaceAntiSpoofing>(antiSetting);
	faceAntiSpoofing->SetThreshold(0.3f, 0.9f); // 设置默认阈值，另外一组阈值为(0.7, 0.55)

	sImg.width = 0;
	sImg.height = 0;
	sImg.channels = 0;
	sImg.data = nullptr;
	LOG_INFO("contructor...........................finished");
}

CIdentifyThread::~CIdentifyThread()
{
	LOG_INFO("decontructor...........................start");
	isExit = true;
	while (isRunning())
	{
		msleep(1);
	}
	LOG_INFO("decontructor...........................finished");
}

void CIdentifyThread::StartRun()
{
	isRun = true;
}

void CIdentifyThread::StopRun()
{
	isRun = false;
}

bool CIdentifyThread::IsInRunning()
{
	return isRun;
}

void CIdentifyThread::Close()
{
	isRun = false;
	isExit = true;
}

void CIdentifyThread::GetIdentifyThreshold(float& threshold) const
{
	threshold = identifyThreshold;
}

void CIdentifyThread::SetIdentifyThreshold(const float threshold)
{
	identifyThreshold = threshold;
}

void CIdentifyThread::EnableAntiSproof(const bool enabled)
{
	enableAntiProof = enabled;
}

void CIdentifyThread::SetCapturedImg(const SeetaImageData& img)
{
	if (sImg.width != img.width || sImg.height != img.height || sImg.channels != img.channels)
	{
		if (sImg.data)
		{
			delete[] sImg.data;
			sImg.data = nullptr;
		}
		sImg.width = img.width;
		sImg.height = img.height;
		sImg.channels = img.channels;
		sImg.data = new unsigned char[img.width * img.height * img.channels];
	}

	memcpy(sImg.data, img.data, img.width * img.height * img.channels);
}

void CIdentifyThread::SetTrackedFaceInfos(const QVector<SeetaFaceInfo>& faceInfos)
{
	trackedFaceInfos = faceInfos;
}

void CIdentifyThread::run()
{
	LOG("run................................start");
	int nret = 0;

	while (!isExit)
	{
		if (isRun)
		{
			// Identify.
			frInfos.clear();
			for (int i = 0; i < trackedFaceInfos.size(); ++i)
			{
				if (isExit)
				{
					return;
				}

				SeetaFaceInfo& faceinfo = trackedFaceInfos[i];
				nret = Recognize(faceinfo);
			}

			// 发送人脸信息给外部
			emit SignalsBroadcastFRInfos(frInfos);

			isRun = false;
		}
		else
		{
			msleep(10);
		}
	}
	LOG("run................................finished");
}

int CIdentifyThread::Recognize(const SeetaFaceInfo& faceinfo)
{
	// 标记人脸位置
	auto points = faceLandmarker->mark(sImg, faceinfo.pos);

	// 判断人脸识别质量
	faceQualityAssessor->feed(sImg, faceinfo.pos, points.data(), 5);
	auto result1 = faceQualityAssessor->query(seeta::BRIGHTNESS);
	auto result2 = faceQualityAssessor->query(seeta::RESOLUTION);
	auto result3 = faceQualityAssessor->query(seeta::CLARITY);
	auto result4 = faceQualityAssessor->query(seeta::INTEGRITY);
	auto result = faceQualityAssessor->query(seeta::POSE_EX);

	if (result.level == 0 || result1.level == 0 || result2.level == 0 || result3.level == 0 || result4.level == 0)
	{
		//LOG("Quality check failed!");
		return -1;
	}

	// 活体检测
	if (enableAntiProof)
	{
		auto status = faceAntiSpoofing->PredictVideo(sImg, faceinfo.pos, points.data());// m_mainthread->m_spoof->Predict(*m_mainthread->m_mainImage, faceinfo.pos, points.data());
		if (status != seeta::FaceAntiSpoofing::REAL)
		{
			LOG("Antispoofing check failed!");
			return -2;
		}
	}


	// 提取人脸数据
	auto cropedFace = faceRecognizer->CropFaceV2(sImg, points.data());
	std::vector<float> cropedFeature;
	cropedFeature.resize(1024);
	faceRecognizer->ExtractCroppedFace(cropedFace, cropedFeature.data());

	// 依次与数据库中人脸信息进行比对
	for (auto& faceInfoInDB : faceInfosInDB)
	{
		if (isExit)
		{
			return -3;
		}

		// 计算提取的人脸与数据库中人脸相似度
		auto sim = faceRecognizer->CalculateSimilarity(cropedFeature.data(), faceInfoInDB.feature.data());
		if (sim >= identifyThreshold) // 返回识别到的人脸信息
		{
			frInfos.push_back({ faceInfoInDB.id, faceInfoInDB.name, faceinfo.pos.x, faceinfo.pos.y,faceinfo.pos.width, faceinfo.pos.height, faceinfo.score, sim });
			//LOG("*************************Find Face In DB***************************");
		}
	}

	return 0;
}

END_NX_NAMESPACE


