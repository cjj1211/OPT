/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   August 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "CVideoThread.h"
#include "CIdentifyThread.h"
#include <CLogger.h>
#include <seeta/FaceDetector.h>
#include <QImage>
#include <format>
#include <QElapsedTimer>


BEGIN_NX_NAMESPACE
CVideoThread::CVideoThread(const QVector<FaceInfo>& faceInfoVec)
	:faceInfoInDB(faceInfoVec)
{
	LOG_INFO("contructor...........................start");
	isCameraReady = InitVideo();
	if (isCameraReady)
	{
		ResetModel();
	}
	qRegisterMetaType<QVector<NxFaceRecognizeInfo>>("QVector<NxFaceRecognizeInfo>)");
	identifyThread = std::make_unique<CIdentifyThread>(faceInfoVec);
	connect(identifyThread.get(), SIGNAL(SignalsBroadcastFRInfos(QVector<NxFaceRecognizeInfo>)), this, SLOT(SlotUpdateFRInfos(QVector<NxFaceRecognizeInfo>)));
	LOG_INFO("contructor...........................finished");
}

CVideoThread::~CVideoThread()
{
	LOG_INFO("decontructor...........................start");
	isExit = true;
	while (isRunning())
	{
		msleep(1);
	}

	LOG_INFO("decontructor...........................finished");
}

bool CVideoThread::IsCameraInited() const
{
	return isCameraReady;
}

bool CVideoThread::IsRunning() const
{
	return isInRunning;
}

void CVideoThread::StartRunning()
{
	capture->open(0); //打开相机
	isInRunning = true;
	isExit = false;
}

void CVideoThread::StopRunning()
{
	isInRunning = false;
}

void CVideoThread::GetIdentifyThreshold(float& threshold) const
{
	identifyThread->GetIdentifyThreshold(threshold);
}

void CVideoThread::SetIdentifyThreshold(const float threshold)
{
	identifyThread->SetIdentifyThreshold(threshold);
}

void CVideoThread::EnableAntiSproof(const bool enabled)
{
	identifyThread->EnableAntiSproof(enabled);
}

void CVideoThread::EnableDrawDetectedFaces(const bool enabled)
{
	isDrawDetectedFaces = enabled;
}

void CVideoThread::EnableDrawRecognizedFaces(const bool enabled)
{
	isDrawRecognizedFaces = enabled;
}

void CVideoThread::EnableDrawName(const bool enabled)
{
	isDrawIdentifiedName = enabled;
}

void CVideoThread::EnableDrawSimilarity(const bool enabled)
{
	isDrawSimilarity = enabled;
}

void CVideoThread::Exit()
{
	isInRunning = false;
	isExit = true;
	capture->release(); //打开相机
}

void CVideoThread::run()
{
	LOG("run................................start");
	if (isCameraReady)
	{
		cv::Mat capturedImg;
		SeetaImageData sImg;
		sImg.height = videoHeight;
		sImg.width = videoWidth;
		sImg.channels = videoImgChannels;
		identifyThread->start();
		identifyThread->StartRun();

		while (!isExit)
		{
			QElapsedTimer timer;
			timer.start();
			if (isInRunning)
			{
				const auto t = timer.elapsed();
				if (t < 30)
				{
					msleep(30 - t);
				}
				timer.restart();

				*capture >> capturedImg;
				if (capturedImg.data == nullptr)
				{
					isExit = true;
					break;
				}

				// 水平镜像
				flip(capturedImg, capturedImg, 2);
				if (capturedImg.channels() == 4)
				{
					cvtColor(capturedImg, capturedImg, cv::COLOR_RGBA2RGB);
				}

				cvtColor(capturedImg, capturedImg, cv::COLOR_BGR2RGB);
				sImg.data = capturedImg.data;
				const auto detectedFaceInfoArray = faceDetector->detect(sImg);
				QVector<NxFaceDetectedInfo> detectedFaceInfos;
				if (detectedFaceInfoArray.size > 0)
				{
					std::lock_guard<QMutex> lock(mutex);
					if (!identifyThread->IsInRunning())
					{
						identifyThread->SetCapturedImg(sImg);

						QVector<SeetaFaceInfo> faceInfos;
						for (int i = 0; i < detectedFaceInfoArray.size; ++i)
						{
							faceInfos.push_back(detectedFaceInfoArray.data[i]);
						}
						identifyThread->SetTrackedFaceInfos(faceInfos);
						identifyThread->StartRun();
					}

					detectedFaceInfos.clear();
					for (int i = 0; i < detectedFaceInfoArray.size; ++i) {
						auto info = detectedFaceInfoArray.data[i];
						detectedFaceInfos.append({ info.pos.x, info.pos.y, info.pos.width, info.pos.height, info.score });
					}

				}

				// 绘制存在于数据库中的人脸位置
				if (isDrawRecognizedFaces)
				{
					for (auto& frInfo : frInfos)
					{
						cv::rectangle(capturedImg, { frInfo.x, frInfo.y, frInfo.width, frInfo.height }, { 0, 255, 0 });
						for (auto& faceInfo : faceInfoInDB)
						{
							if (frInfo.id == faceInfo.id)
							{
								std::string drawText = "";
								if (isDrawIdentifiedName && isDrawSimilarity) {
									drawText = std::format("{} {}", faceInfo.name.toStdString(), frInfo.similar);
								}
								else if (isDrawIdentifiedName) {
									drawText = faceInfo.name.toStdString();
								}
								else if (isDrawSimilarity) {
									drawText = frInfo.similar;
								}
								else {
									break;
								}
								cv::putText(capturedImg, std::format("{} {}", faceInfo.name.toStdString(), frInfo.similar), { frInfo.x, frInfo.y }, cv::FONT_HERSHEY_SIMPLEX, 1, { 250, 255,0 });
							}
						}
					}
				}

				// 绘制所有识别到的人脸
				if (isDrawDetectedFaces)
				{
					for (int i = 0; i < detectedFaceInfoArray.size; ++i)
					{
						auto pos = detectedFaceInfoArray.data[i].pos;
						cv::rectangle(capturedImg, { pos.x, pos.y, pos.width, pos.height }, { 0, 0, 255 });
					}
				}

#pragma warning(disable:4267)
				const QImage image(static_cast<const unsigned char*>(capturedImg.data), capturedImg.cols, capturedImg.rows, capturedImg.step, QImage::Format_RGB888);
				emit SignalBroadcastFrame(image, detectedFaceInfos, frInfos);
				msleep(30);
			}
			else
			{
				QThread::msleep(30);
			}
		}

		// 结束当前线程任务
		identifyThread->StopRun();
	}
	LOG("run................................finished");
}

void CVideoThread::ResetModel()
{
	constexpr auto deviceType = SEETA_DEVICE_CPU;

	seeta::ModelSetting fdSetting;
	fdSetting.set_device(deviceType);
	fdSetting.append("./models/face_detector.csta");
	faceDetector = std::make_unique<seeta::FaceDetector>(fdSetting);
	faceDetector->set(seeta::v6::FaceDetector::PROPERTY_THRESHOLD, 0.5);
}

bool CVideoThread::InitVideo()
{
	capture = std::make_unique<cv::VideoCapture>();
	//设置编码格式，//https://www.fourcc.org/codecs.php
	//更多属性设置参考：https://docs.opencv.org/4.5.0/d9/df8/tutorial_root.html
	capture->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
	constexpr int deviceID = 0; //相机设备号
	capture->open(deviceID); //打开相机

	if (!capture->isOpened()) //判断相机是否打开
	{
		LOG_WARN("ERROR!!Unable to open camera\n");
		return false;
	}

	cv::Mat img;
	*capture >> img;
	videoWidth = img.cols;
	videoHeight = img.rows;
	videoImgChannels = img.channels();
	return true;
}

void CVideoThread::SlotUpdateFRInfos(QVector<NxFaceRecognizeInfo> recognizedInfos)
{
	frInfos.clear();
	frInfos = recognizedInfos;
}


END_NX_NAMESPACE


