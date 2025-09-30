/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   August 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __CFEATURE_DETECTOR_H
#define __CFEATURE_DETECTOR_H

#include "NxFaceIdenfierGlobal.h"
#include <memory>
#include <vector>
#include <QImage>

namespace seeta::v6
{
    class FaceDetector;
    class FaceLandmarker;
    class FaceRecognizer;
}

BEGIN_NX_NAMESPACE

/** 人脸特征提取器.
 */
class NXFACEIDENTIFIER_EXPORT CFeatureDetector
{
public:
    /**
     * 构造函数.
     * 
     */
    CFeatureDetector();

    /**
     * 析构函数.
     * 
     */
    ~CFeatureDetector();

    /**
     * 从输入图像中提取人脸特征.
     * 
     * @param imgPath 图像路径
     * @param feature 人脸特征数组
     * @return 
     */
    bool ExtractFeature(const std::string& imgPath, std::vector<float>& feature);

    /**
     * 从输入图像中提取人脸特征.
     * 
     * @param img 输入图像
     * @param feature 人脸特征
     * @return 是否成功
     */
    bool ExtractFeature(const QImage& img, std::vector<float>& feature);
private:
    seeta::v6::FaceDetector* faceDetector;
    seeta::v6::FaceLandmarker* faceLandmarker;
    seeta::v6::FaceRecognizer* faceRecognizer;
};

END_NX_NAMESPACE

#endif

