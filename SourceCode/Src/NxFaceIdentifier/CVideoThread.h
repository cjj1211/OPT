/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   August 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __CVIDEO_THREAD_H
#define __CVIDEO_THREAD_H

#include "NxFaceIdenfierGlobal.h"

#include <QThread>
#include <QMutex>
#include <QImage>
#include <QVector>

namespace seeta::v6 {
	class FaceDetector;
}

namespace cv {
	class VideoCapture;
}

BEGIN_NX_NAMESPACE

class CIdentifyThread;
class CVideoThread : public QThread
{
	Q_OBJECT
public:
	explicit CVideoThread(const QVector<FaceInfo>& faceInfoVec);
	~CVideoThread();

	bool IsCameraInited() const;
	bool IsRunning() const;
	void StartRunning();
	void StopRunning();
	void GetIdentifyThreshold(float& threshold) const;
	void SetIdentifyThreshold(const float threshold);
	void EnableAntiSproof(const bool enabled);
	void EnableDrawDetectedFaces(const bool enabled);
	void EnableDrawRecognizedFaces(const bool enabled);
	void EnableDrawName(const bool enabled);
	void EnableDrawSimilarity(const bool enbaled);
	void Exit();

signals:
	void SignalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>);

private slots:
	void SlotUpdateFRInfos(QVector<NxFaceRecognizeInfo>);

protected:
	void run() override;

private:
	void ResetModel();
	bool InitVideo();
	QMutex mutex;

	std::unique_ptr<CIdentifyThread> identifyThread;
	std::unique_ptr<cv::VideoCapture> capture;
	std::unique_ptr<seeta::v6::FaceDetector> faceDetector;
	int videoWidth{ 0 };
	int videoHeight{ 0 };
	int videoImgChannels{ 0 };
	int minFaceSize{ 100 };
	QVector<NxFaceRecognizeInfo> frInfos;
	QVector<FaceInfo> faceInfoInDB;

	bool isInRunning{ false };
	bool isExit{ false };
	bool isCameraReady{ false };
	bool isDrawIdentifiedName{ false };
	bool isDrawSimilarity{ false };
	bool isDrawDetectedFaces{ false };
	bool isDrawRecognizedFaces{ false };
};
END_NX_NAMESPACE

#endif