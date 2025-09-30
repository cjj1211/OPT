#pragma once
#include <CLogger.h>
#include <QtCore/qglobal.h>
#include <QVector>

#ifndef BUILD_STATIC
# if defined(NXFACEIDENTIFIER_LIB)
#  define NXFACEIDENTIFIER_EXPORT Q_DECL_EXPORT
# else
#  define NXFACEIDENTIFIER_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXFACEIDENTIFIER_EXPORT
#endif

using namespace NX;

/** 录入的人脸信息.
 */
typedef struct FaceInfo {
    /** 数据库中存储的人脸唯一标识.  */
    int id;

    /** 人脸图像宽度 */
    int width;

    /** 人脸图像高度 */
    int height;

    /** 人脸标注名称 */
    QString name;

    /** 人脸特征信息 */
    std::vector<float> feature;
}FaceInfo;

/** 识别到的人脸信息.
 */
typedef struct NxFaceRecognizeInfo {
    /** 对应数据库中存储的人脸信息ID. */
    int id; 

    /** 人脸标注名称 */
    QString identityUid;

    /** 识别到的人脸截图左上角在图片中的x坐标. */
    int x;

    /** 识别到的人脸截图左上角在图片中的y坐标. */
    int y;

    /** 识别出的人脸截图宽度. */
    int width;

    /** 识别出的人脸截图高度. */
    int height;

    /** 识别到的人脸评分. */
    float score { 0};

    /** 识别出的人脸相似度. */
    float similar;
}NxFaceRecognizeInfo;

/** 识别到的人脸信息.
 */
typedef struct NxFaceDetectedInfo {
    /** 识别到的人脸截图左上角在图片中的x坐标. */
    int x;

    /** 识别到的人脸截图左上角在图片中的y坐标. */
    int y;

    /** 识别出的人脸截图宽度. */
    int width;

    /** 识别出的人脸截图高度. */
    int height;

    /** 识别到的人脸评分. */
    float score;
}NxFaceDetectedInfo;
