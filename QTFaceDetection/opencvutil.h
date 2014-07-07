#ifndef OPENCVUTIL_H
#define OPENCVUTIL_H

#include <opencv2/core/core.hpp>
#include <QImage>

class OpenCVUtil
{
public:
    static QImage CVImgToQTImg(const cv::Mat &opencvImg);
};

#endif // OPENCVUTIL_H
