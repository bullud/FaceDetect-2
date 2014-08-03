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

QListWidgetItem *OpenCVUtil::CreateFaceItem(const cv::Mat &face, int id)
{
    QListWidgetItem *item = new QListWidgetItem(QIcon(QPixmap::fromImage(CVImgToQTImg(face))), "");
    item->setData(Qt::UserRole, id);
    return item;
}

void OpenCVUtil::AddFaceItem(QListWidget *listBox, const cv::Mat &face, int id)
{
    vector<int> faceIds;
    for (int i = 0; i < listBox->count(); ++i)
        faceIds.push_back(listBox->item(i)->data(Qt::UserRole).toInt());

    if (faceIds.end() == find(faceIds.begin(), faceIds.end(), id))
    {
        listBox->addItem(CreateFaceItem(face, id));
        faceIds.push_back(id);
    }
}
