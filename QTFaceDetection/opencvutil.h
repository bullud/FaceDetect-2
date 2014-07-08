#ifndef OPENCVUTIL_H
#define OPENCVUTIL_H

#include <opencv2/core/core.hpp>
#include <QImage>
#include <QListWidgetItem>

class OpenCVUtil
{
public:
    static QImage CVImgToQTImg(const cv::Mat &opencvImg);
    static QListWidgetItem *CreateFaceItem(const cv::Mat &face);
};

#endif // OPENCVUTIL_H
