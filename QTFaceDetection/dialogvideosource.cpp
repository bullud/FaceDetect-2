#include "dialogvideosource.h"
#include "ui_dialogvideosource.h"
#include "facedetection.h"
#include <QFileDialog>
#include <QMessageBox>
#include <cassert>

DialogVideoSource::DialogVideoSource(
        QWidget *parent,
        std::unique_ptr<cv::VideoCapture> &capture,
        DataContext &dataContext) :
    QDialog(parent),
    ui(new Ui::DialogVideoSource),
    capture_(capture),
    dataContext_(dataContext),
    videoSource_(dataContext.GetSource())
{
    ui->setupUi(this);

    switch (dataContext_.GetSource())
    {
    case CAMERA:
        ui->radioButtonCamera->setChecked(true);
        ui->lineEditVideoFile->setEnabled(false);
        ui->pushButtonVideoFile->setEnabled(false);
        break;
    case VIDEOFILE:
        ui->radioButtonFile->setChecked(true);
        ui->lineEditVideoFile->setEnabled(true);
        ui->pushButtonVideoFile->setEnabled(true);
        break;
    default:
        assert(0);
    }

    ui->lineEditVideoFile->setText(dataContext_.GetVideoFilePath());
}

DialogVideoSource::~DialogVideoSource()
{
    delete ui;
}

void DialogVideoSource::on_radioButtonCamera_clicked()
{
    videoSource_ = CAMERA;
    ui->lineEditVideoFile->setEnabled(false);
    ui->pushButtonVideoFile->setEnabled(false);
}

void DialogVideoSource::on_radioButtonFile_clicked()
{
    videoSource_ = VIDEOFILE;
    ui->lineEditVideoFile->setEnabled(true);
    ui->pushButtonVideoFile->setEnabled(true);
}

void DialogVideoSource::on_pushButtonVideoFile_clicked()
{
    QFileDialog fd(this, QStringLiteral("选择视频文件"), ".", QStringLiteral("视频文件(*.avi)"));
    fd.setFileMode(QFileDialog::AnyFile);
    if(fd.exec() != QDialog::Accepted)
        return;

    QString path(fd.selectedFiles()[0]);
    dataContext_.SetVideoFilePath(path);
    ui->lineEditVideoFile->setText(path);
}

void DialogVideoSource::accept()
{
    dataContext_.SetVideoFilePath(ui->lineEditVideoFile->text());

    if (dataContext_.GetSource() == CAMERA && videoSource_ == CAMERA)
    {
        QDialog::accept();
        return;
    }

    std::unique_ptr<cv::VideoCapture> newSource(
        videoSource_ == CAMERA ?
        new cv::VideoCapture(0) :
        new cv::VideoCapture(ui->lineEditVideoFile->text().toStdString()));
    if (!newSource->isOpened())
    {
        QMessageBox::critical(this,
                              QStringLiteral("错误"),
                              QStringLiteral("无法读取视频文件！"),
                              QMessageBox::Yes,
                              QMessageBox::Yes);
        return;
    }

    capture_.swap(newSource);
    dataContext_.SetSource(videoSource_);
    QDialog::accept();
}
