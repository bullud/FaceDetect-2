#include "opencvutil.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <QIcon>
#include <QPixmap>
#include <vector>
#include <algorithm>

using std::vector;
using std::find;

QImage OpenCVUtil::CVImgToQTImg(const cv::Mat &opencvImg)
{
    cv::cvtColor(opencvImg, opencvImg, CV_BGR2RGB);
    return QImage((const uchar*)opencvImg.data, opencvImg.cols, opencvImg.rows, QImage::Format_RGB888);
}

QListWidgetItem *OpenCVUtil::CreateFaceItem(const cv::Mat &face)
{
    return new QListWidgetItem(QIcon(QPixmap::fromImage(CVImgToQTImg(face))), "");
}

void OpenCVUtil::AddFaceItem(QListWidget *listBox, const cv::Mat &face, int id)
{
    static vector<int> s_FaceIds;
    if (s_FaceIds.end() == find(s_FaceIds.begin(), s_FaceIds.end(), id))
    {
        listBox->addItem(CreateFaceItem(face));
        s_FaceIds.push_back(id);
    }
}
