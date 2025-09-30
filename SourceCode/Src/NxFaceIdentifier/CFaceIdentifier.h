/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
*  作者 : 宋志杰 zhijie.song@neuroxess.com
*  修改记录:
*  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __NX_FACE_IDENTIFIER_H
#define __NX_FACE_IDENTIFIER_H

#include "NxFaceIdenfierGlobal.h"

#include <QImage>
#include <QString>
#include <QVector>
#include <memory>

BEGIN_NX_NAMESPACE

class CVideoThread;

/** 人脸识别器.
 */
class NXFACEIDENTIFIER_EXPORT CFaceIdentifier : public QObject
{
    Q_OBJECT
public:
    /**
     * 构造函数.
     *
     * @param[in] faceInfoVec 需要匹配的人脸信息列表
     */
    CFaceIdentifier(const QVector<FaceInfo>& faceInfoVec);

    /**
     * 析构.
     *
     */
    ~CFaceIdentifier();

    /**
     * 获取人脸识别的阈值.
     *
     * @param[out] threshold 阈值，默认为0.7, 取值范围0~1
     */
    void GetIdentifyThreshold(float& threshold) const;

    /**
     * 设置人脸识别的阈值.
     *
     * @param[in] threshold 阈值，默认为0.7, 取值范围0~1
     */
    void SetIdentifyThreshold(const float threshold);

    /**
     * 开启或关闭活体检测功能.
     *
     * @param[in] enabled 是否开启
     */
    void EnableAntiSproof(const bool enabled);

    /**
     * 开始采集图像并进行人脸识别.
     *
     */
    void StartIdentify();

    /**
     * 停止采集图像和人脸识别.
     *
     */
    void FinishIdentify();

    /**
     * 开启或关闭绘制检测到的人脸区域.
     *
     * @param[in] enabled 开启或关闭
     */
    void EnableDrawDetectedFaces(const bool enabled);

    /**
     * 开启或关闭是否要绘制.
     *
     * @param[in] enabled
     */
    void EnableDrawRecognizedFaces(const bool enabled);

    /**
     * 开启或者关闭绘制名称功能.
     *
     * @param[in] enabled 开启或关闭
     */
    void EnableDrawName(const bool enabled);

    /**
     * 开启或关闭绘制相似度.
     *
     * @param[in] enbaled 开启或关闭
     */
    void EnableDrawSimilarity(const bool enbaled);

    /**
     * 摄像头是否已开启.
     *
     * @return 摄像头开启状态
     */
    bool IsCameraOpen() const;
signals:
    /**
     * \brief 发送采集到的图像的信号.
     * \param capturedImg 捕获的图片
     * \param detectedFaceInfos 识别到的人脸信息
     * \param recognizeInfos 认出的人脸
     */
    void SignalBroadcastFrame(QImage capturedImg, QVector<NxFaceDetectedInfo> detectedFaceInfos, QVector<NxFaceRecognizeInfo> recognizeInfos);

private:
    CVideoThread* videoThread;
};

END_NX_NAMESPACE

#endif