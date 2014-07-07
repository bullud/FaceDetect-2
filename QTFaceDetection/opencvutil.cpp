#include "opencvutil.h"
#include <opencv2/imgproc/imgproc.hpp>

QImage OpenCVUtil::CVImgToQTImg(const cv::Mat &opencvImg)
{
    if(opencvImg.channels() == 3)
    {
        //cvt Mat BGR 2 QImage RGB
        cv::Mat rgb;
        cv::cvtColor(opencvImg, rgb, CV_BGR2RGB);
        return QImage((const unsigned char*)(rgb.data),
                    rgb.cols, rgb.rows,
                    rgb.cols * rgb.channels(),
                    QImage::Format_RGB888);
    }
    else
    {
        return QImage((const unsigned char*)(opencvImg.data),
                    opencvImg.cols, opencvImg.rows,
                    opencvImg.cols * opencvImg.channels(),
                    QImage::Format_RGB888);
    }
}
