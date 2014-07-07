#include "opencvutil.h"

QImage OpenCVUtil::CVImgToQTImg(const cv::Mat &opencvImg)
{
    return QImage((const uchar*)opencvImg.data, opencvImg.cols, opencvImg.rows, QImage::Format_RGB888);
}
