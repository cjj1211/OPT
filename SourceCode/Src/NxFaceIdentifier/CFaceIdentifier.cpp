/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   August 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "CFaceIdentifier.h"
#include "CVideoThread.h"
#include <opencv2/opencv.hpp>
#include <QDebug>

BEGIN_NX_NAMESPACE

CFaceIdentifier::CFaceIdentifier(const QVector<FaceInfo>& faceInfoVec)
{
    videoThread = new CVideoThread(faceInfoVec);// std::make_unique<CVideoThread>(faceInfoVec);

    qRegisterMetaType<QVector<NxFaceDetectedInfo>>("QVector<NxFaceDetectedInfo>)");
    connect(videoThread, SIGNAL(SignalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)), this, SIGNAL(SignalBroadcastFrame(QImage, QVector<NxFaceDetectedInfo>, QVector<NxFaceRecognizeInfo>)));

}

CFaceIdentifier::~CFaceIdentifier()
{
    if (videoThread->IsRunning())
    {
        videoThread->StopRunning();
    }

    // 等待线程结束
    if (!videoThread->isFinished())
    {
        QThread::msleep(2);
    }

    delete videoThread;
}

void CFaceIdentifier::GetIdentifyThreshold(float& threshold) const
{
    videoThread->GetIdentifyThreshold(threshold);
}

void CFaceIdentifier::SetIdentifyThreshold(const float threshold)
{
    videoThread->SetIdentifyThreshold(threshold);
}

void CFaceIdentifier::EnableAntiSproof(const bool enabled)
{
    videoThread->EnableAntiSproof(enabled);
}

void CFaceIdentifier::StartIdentify()
{
    if (!videoThread->IsRunning()) {

        videoThread->StartRunning();
        videoThread->start();
    }
}

void CFaceIdentifier::FinishIdentify()
{
    videoThread->Exit();
    // 判断线程是否已完成，未完成则等待
    while (videoThread->isRunning())
    {
        QThread::msleep(1);
    }
}

void CFaceIdentifier::EnableDrawDetectedFaces(const bool enabled)
{
    videoThread->EnableDrawDetectedFaces(enabled);
}

void CFaceIdentifier::EnableDrawRecognizedFaces(const bool enabled)
{
    videoThread->EnableDrawRecognizedFaces(enabled);
}

void CFaceIdentifier::EnableDrawName(const bool enabled)
{
    videoThread->EnableDrawName(enabled);
}

void CFaceIdentifier::EnableDrawSimilarity(const bool enbaled)
{
    videoThread->EnableDrawSimilarity(enbaled);
}

bool CFaceIdentifier::IsCameraOpen() const
{
    return videoThread->IsCameraInited();
}

END_NX_NAMESPACE
