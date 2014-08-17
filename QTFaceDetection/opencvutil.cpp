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
    if(opencvImg.channels() == 1)
    {
        static QVector<QRgb>  sColorTable;

        // only create our color table once
        if ( sColorTable.isEmpty() )
        {
           for ( int i = 0; i < 256; ++i )
              sColorTable.push_back( qRgb( i, i, i ) );
        }

        QImage image( opencvImg.data, opencvImg.cols, opencvImg.rows, opencvImg.step, QImage::Format_Indexed8 );
        image.setColorTable( sColorTable );
        return image;
    }
    else
    {
        cv::cvtColor(opencvImg, opencvImg, CV_BGR2RGB);
        return QImage((const uchar*)opencvImg.data, opencvImg.cols, opencvImg.rows, QImage::Format_RGB888);
    }
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
        auto item = CreateFaceItem(face, id);
        listBox->addItem(item);
        listBox->setCurrentItem(item);
        faceIds.push_back(id);
    }
}
