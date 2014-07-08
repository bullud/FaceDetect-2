#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "facedetection.h"
#include "datacontext.h"

#include <QMainWindow>
#include <QTimer>

#include <opencv2/highgui/highgui.hpp>

#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    std::unique_ptr<Ui::MainWindow> ui;

private:
    DataContext dataContext_;
    cv::VideoCapture capture_;
    FaceDetection faceDetection_;
    QTimer timer_;

private slots:
    void OnToggleMode();
    void OnTimeout();
};

#endif // MAINWINDOW_H
