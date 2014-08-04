﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "facedetection.h"
#include "datacontext.h"

#include <QMainWindow>
#include <QActionGroup>
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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private:
    DataContext dataContext_;
    FaceDetection faceDetection_;
    QTimer timer_;
    std::unique_ptr<cv::VideoCapture> capture_;
    std::unique_ptr<QActionGroup> actionGroup;
    size_t frame_index_;
    size_t createdTemplates;
    bool bfirst_;
    cv::VideoWriter *videoWriter_;

private:
    void UseCamera();

private slots:
    void OnTimeout();
    void selectMode(QAction *action);
    void on_menuFileExit_triggered();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButtonStartStopTemplate_clicked();
    void on_actionSetParam_triggered();
    void deleteItem();
    void on_actionVideoSource_triggered();
};

#endif // MAINWINDOW_H
