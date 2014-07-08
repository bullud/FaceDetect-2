#ifndef FACEDETECTION_H
#define FACEDETECTION_H

#include <opencv2/core/core.hpp>

class FaceDetection
{
public:
    cv::Mat CreateFaceTemplate(const cv::Mat &frame);
    cv::Mat &DetectFace(cv::Mat &frame, const cv::Mat &face);

    bool IsEquivalent(const cv::Mat &face0, const cv::Mat &face1);
};

#endif // FACEDETECTION_H
