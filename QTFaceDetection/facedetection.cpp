#include "facedetection.h"

cv::Mat FaceDetection::CreateFaceTemplate(const cv::Mat &/*frame*/)
{
    return cv::Mat();
}

cv::Mat &FaceDetection::DetectFace(cv::Mat &frame, const cv::Mat &/*face*/)
{
    return frame;
}

bool FaceDetection::IsEquivalent(const cv::Mat &/*face0*/, const cv::Mat &/*face1*/)
{
    return true;
}
