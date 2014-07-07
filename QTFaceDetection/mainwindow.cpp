#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencvutil.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer_(this),
    capture_(0)
{
    ui->setupUi(this);

    ui->radioButtonCreation->setChecked(dataContext_.GetMode() == CREATION);

    connect(
        ui->radioButtonCreation,
        SIGNAL(clicked()),
        this,
        SLOT(OnToggleMode()));
    connect(
        ui->radioButtonDetection,
        SIGNAL(clicked()),
        this,
        SLOT(OnToggleMode()));
    connect(
        &timer_,
        SIGNAL(timeout()),
        this,
        SLOT(OnTimeout())
        );

    if (!capture_.isOpened())
    {
        QMessageBox::critical(
            NULL,
            "Critical",
            "The camera can't be opened!",
            QMessageBox::Yes,
            QMessageBox::Yes);
    }

    timer_.start(50);
}

MainWindow::~MainWindow()
{
}

void MainWindow::OnToggleMode()
{
    dataContext_.SetMode(
        ui->radioButtonCreation->isChecked()?
        CREATION : DETECTION);
}

void MainWindow::OnTimeout()
{
    if (!capture_.isOpened()) return;

    cv::Mat frame;
    capture_ >> frame;

    cv::Mat &displayFrame = faceDetection_.DetectFace(frame, cv::Mat());

    QImage qimage(OpenCVUtil::CVImgToQTImg(displayFrame));
    ui->canvas->setPixmap(QPixmap::fromImage(qimage));
}
