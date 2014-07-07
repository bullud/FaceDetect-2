#include "facedetection.h"

FaceDetection::FaceDetection()
{
}

cv::Mat FaceDetection::CreateFaceTemplate(const cv::Mat &/*frame*/)
{
    return cv::Mat();
}

cv::Mat &FaceDetection::DetectFace(cv::Mat &frame, const cv::Mat &/*face*/)
{
    return frame;
}
