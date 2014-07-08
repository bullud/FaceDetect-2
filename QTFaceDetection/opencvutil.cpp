#include "opencvutil.h"
#include <opencv2/imgproc/imgproc.hpp>

QImage OpenCVUtil::CVImgToQTImg(const cv::Mat &opencvImg)
{
    cv::cvtColor(opencvImg, opencvImg, CV_BGR2RGB);
    return QImage((const uchar*)opencvImg.data, opencvImg.cols, opencvImg.rows, QImage::Format_RGB888);
}
