#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencvutil.h"
#include <QMessageBox>
#include <QDebug>
#include <cassert>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer_(this),
    capture_(0),
    actionGroup(new QActionGroup(this))
{
    ui->setupUi(this);

    actionGroup->addAction(ui->faceTemplate);
    actionGroup->addAction(ui->faceRecognition);
    actionGroup->addAction(ui->videoRecord);

    if (!capture_.isOpened())
    {
        QMessageBox::critical(
            NULL,
            "Critical",
            "The camera can't be opened!",
            QMessageBox::Yes,
            QMessageBox::Yes);
    }

    connect(actionGroup.get(), SIGNAL(triggered(QAction*)), this, SLOT(selectMode(QAction*)));

    switch (dataContext_.GetMode())
    {
    case TEMPLATE:
        ui->faceTemplate->trigger();
        break;
    case DETECTION:
        ui->faceRecognition->trigger();
        break;
    case RECORD:
        ui->videoRecord->trigger();
        break;
    default:
        assert(0);
    }

    connect(&timer_, SIGNAL(timeout()), this, SLOT(OnTimeout()));
    timer_.start(50);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selectMode(QAction *action)
{
    if (action == ui->faceTemplate)
    {
        dataContext_.SetMode(TEMPLATE);
        ui->groupBoxVideoRecord->hide();
        ui->groupBoxFaceTempalte->show();
    }
    else if (action == ui->faceRecognition)
    {
        dataContext_.SetMode(DETECTION);
        ui->groupBoxVideoRecord->hide();
        ui->groupBoxFaceTempalte->show();
    }
    else if (action == ui->videoRecord)
    {
        dataContext_.SetMode(RECORD);
        ui->groupBoxVideoRecord->show();
        ui->groupBoxFaceTempalte->hide();
    }
    else
        assert(0);

    qDebug() << dataContext_.GetMode();
}

void MainWindow::OnTimeout()
{
    if (!capture_.isOpened()) return;

    // Capture one frame from the camera
    cv::Mat frame;
    capture_ >> frame;

    // Render the frame
    ui->canvas->setPixmap(QPixmap::fromImage(OpenCVUtil::CVImgToQTImg(faceDetection_.DetectFace(frame, cv::Mat()))));
}

void MainWindow::on_menuFileExit_triggered()
{
    QWidget::close();
}
