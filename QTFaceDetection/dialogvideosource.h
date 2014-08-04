void on_pushButtonVideoFile_clicked();
#ifndef DIALOGVIDEOSOURCE_H
#define DIALOGVIDEOSOURCE_H

#include "datacontext.h"
#include <opencv2/highgui/highgui.hpp>
#include <QDialog>
#include <QString>
#include <memory>

namespace Ui {
class DialogVideoSource;
}

class DialogVideoSource : public QDialog
{
    Q_OBJECT

public:
    DialogVideoSource(
            QWidget *parent,
            std::unique_ptr<cv::VideoCapture> &capture,
            DataContext &dataContext);
    ~DialogVideoSource();

private slots:
    void on_radioButtonCamera_clicked();
    void on_radioButtonFile_clicked();
    void on_pushButtonVideoFile_clicked();
    void accept();

private:
    Ui::DialogVideoSource *ui;
    std::unique_ptr<cv::VideoCapture> &capture_;
    DataContext &dataContext_;
    VIDEOSOURCE videoSource_;
};

#endif // DIALOGVIDEOSOURCE_H
