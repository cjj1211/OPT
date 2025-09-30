#include "CFeatureDetector.h"
#include <CLogger.h>
#include <seeta/FaceDetector.h>
#include <seeta/FaceLandmarker.h>
#include <seeta/FaceRecognizer.h>
#include <opencv2/opencv.hpp>

BEGIN_NX_NAMESPACE

CFeatureDetector::CFeatureDetector()
{
    auto deviceType = SEETA_DEVICE_CPU;
    // Seeta Face
    seeta::ModelSetting fdSetting;
    fdSetting.set_device(deviceType);
    fdSetting.append("./models/face_detector.csta");
    faceDetector = new seeta::FaceDetector(fdSetting); //std::make_unique<seeta::FaceDetector>(fdSetting);

    seeta::ModelSetting flSetting;
    flSetting.set_device(deviceType);
    flSetting.append("./models/face_landmarker_pts5.csta");
    faceLandmarker = new seeta::FaceLandmarker(flSetting); //  std::make_unique<seeta::FaceLandmarker>(flSetting);

    seeta::ModelSetting frSetting;
    frSetting.set_id(0);
    frSetting.append("./models/face_recognizer.csta");
    frSetting.set_device(deviceType);
    faceRecognizer = new seeta::FaceRecognizer(frSetting);// std::make_unique<seeta::FaceRecognizer>(frSetting);
}

CFeatureDetector::~CFeatureDetector()
{
    delete faceRecognizer;
    delete faceLandmarker;
    delete faceDetector;
}

bool CFeatureDetector::ExtractFeature(const std::string& imgPath, std::vector<float>& feature)
{
    // TODO 需要检查文件是否读取成功

    cv::Mat mat = cv::imread(imgPath);

    SeetaImageData simg;
    simg.height = mat.rows;
    simg.width = mat.cols;
    simg.channels = mat.channels();
    simg.data = mat.data;

    auto faces = faceDetector->detect(simg);

    if (faces.size <= 0)
    {
        std::cout << "no face detected in" << std::endl;
        return false;
    }

    SeetaPointF points[5];
    faceLandmarker->mark(simg, faces.data[0].pos, points);
    faceRecognizer->Extract(simg, points, feature.data());
    return true;
}

bool CFeatureDetector::ExtractFeature(const QImage& image, std::vector<float>& feature)
{
    cv::Mat mat;
    switch (image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.bits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.bits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.bits(), image.bytesPerLine());
        break;
    }

    SeetaImageData simg;
    simg.height = mat.rows;
    simg.width = mat.cols;
    simg.channels = mat.channels();
    simg.data = mat.data;

    auto faces = faceDetector->detect(simg);

    if (faces.size <= 0)
    {
        std::cout << "no face detected in" << std::endl;
        return false;
    }

    SeetaPointF points[5];
    faceLandmarker->mark(simg, faces.data[0].pos, points);
    faceRecognizer->Extract(simg, points, feature.data());
    return true;
}

END_NX_NAMESPACE