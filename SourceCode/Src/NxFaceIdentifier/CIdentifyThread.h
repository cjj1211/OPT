/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   August 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __CIDENTIFY_THREAD_H
#define __CIDENTIFY_THREAD_H

#include "NxFaceIdenfierGlobal.h"
#include <QThread>

#include <seeta/CTrackingFaceInfo.h>
#include <seeta/FaceAntiSpoofing.h>
#include <seeta/FaceDetector.h>
#include <seeta/FaceLandmarker.h>
#include <seeta/QualityAssessor.h>
#include <seeta/QualityOfPoseEx.h>
#include <seeta/FaceRecognizer.h>
#include <opencv2/opencv.hpp>
#include <QThread>
#include <QMutex>
#include <QImage>
#include <QVector>


namespace cv {
    class Mat;
}


BEGIN_NX_NAMESPACE

class CIdentifyThread:public QThread
{
    Q_OBJECT
public:
    explicit CIdentifyThread(const QVector<FaceInfo>& faceInfoVec);
    ~CIdentifyThread();

	void StartRun();
	void StopRun();
	bool IsInRunning();
	void Close();
	void GetIdentifyThreshold(float& threshold) const;
	void SetIdentifyThreshold(const float threshold);
	void EnableAntiSproof(const bool enabled);
	void SetCapturedImg(const SeetaImageData& img);

	/**
	 * 设置从视频中追踪到的人脸信息.
	 * 
	 * @param faceInfos 人脸信息列表
	 */
	void SetTrackedFaceInfos(const QVector<SeetaFaceInfo>& faceInfos);

signals:
	void SignalsBroadcastFRInfos(QVector<NxFaceRecognizeInfo>);

protected:
	void run() override;

private:
	int Recognize(const SeetaFaceInfo& faceinfo);//

	volatile bool isRun{ false };
	volatile bool isExit { false };
	bool enableAntiProof { true };
	float identifyThreshold{ 0.7f };
	
	SeetaImageData sImg;
	QVector<SeetaFaceInfo> trackedFaceInfos;
	QVector<FaceInfo> faceInfosInDB; // 设置数据库中存储的人脸信息列表
	QVector<NxFaceRecognizeInfo> frInfos;

	std::unique_ptr<cv::Mat> capturedImg;
	std::unique_ptr<seeta::FaceDetector> faceDetector;
	std::unique_ptr<seeta::FaceLandmarker> faceLandmarker;
	std::unique_ptr<seeta::FaceAntiSpoofing> faceAntiSpoofing;
	std::unique_ptr<seeta::QualityAssessor> faceQualityAssessor;
	std::unique_ptr<seeta::FaceRecognizer> faceRecognizer;
	seeta::QualityOfPoseEx* poseEx;
	QMutex mutex;
};

END_NX_NAMESPACE

#endif