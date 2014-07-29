#ifndef OPENCVUTIL_H
#define OPENCVUTIL_H

#include <opencv2/core/core.hpp>
#include <QImage>
#include <QListWidget>
#include <QListWidgetItem>

class OpenCVUtil
{
public:
    static QImage CVImgToQTImg(const cv::Mat &opencvImg);
    static QListWidgetItem *CreateFaceItem(const cv::Mat &face);
    static void AddFaceItem(QListWidget *listBox, const cv::Mat &face, int id);
};

#endif // OPENCVUTIL_H
