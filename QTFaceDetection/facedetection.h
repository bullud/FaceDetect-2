#ifndef FACEDETECTION_H
#define FACEDETECTION_H

#include <opencv2/core/core.hpp>

class FaceDetection
{
public:
    cv::Mat CreateFaceTemplate(const cv::Mat &frame);
    cv::Mat &DetectFace(cv::Mat &frame, const cv::Mat &face);
};

#endif // FACEDETECTION_H
