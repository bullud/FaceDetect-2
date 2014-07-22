#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencvutil.h"
#include <QMessageBox>
#include <QFileDialog>
#include <string>
#include <cassert>

using std::string;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer_(this),
    capture_(0),
    actionGroup(new QActionGroup(this)),
    videoWriter_(nullptr)
{
    ui->setupUi(this);

    actionGroup->addAction(ui->faceTemplate);
    actionGroup->addAction(ui->faceRecognition);
    actionGroup->addAction(ui->videoRecord);

    if (!capture_.isOpened())
    {
        QMessageBox::critical(
            this,
            QStringLiteral("错误"),
            QStringLiteral("无法打开摄像头"),
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

    if (dataContext_.GetRecordStatus())
    {
        ui->pushButton_2->setText(QStringLiteral("停止录像"));
    }
    else
    {
        ui->pushButton_2->setText(QStringLiteral("开始录像"));
    }

    connect(&timer_, SIGNAL(timeout()), this, SLOT(OnTimeout()));
    timer_.start(50);
}

MainWindow::~MainWindow()
{
    delete videoWriter_;
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
}

void MainWindow::OnTimeout()
{
    if (!capture_.isOpened()) return;

    // Capture one frame from the camera
    cv::Mat frame;
    capture_ >> frame;

    if (dataContext_.GetRecordStatus())
    {
        assert(videoWriter_ != nullptr);
        assert(dataContext_.GetMode() == RECORD);
        *videoWriter_ << frame;
    }
	
	/**
	 * just show how to use FaceDetecction class:
	 * option-1: just for test
	 * option-2: 人脸建模模式
	 * option-3: 人脸识别模式
	 */
	 // option-1: just detect/track faces
    /*
    faceDetection_.DetectFace(frame, frame_index_);
    */
    // option-2: create face templates automatically
    /*
    if(faceDetection_.CreateFaceTemplate(frame, frame_index_, bfirst_))
    {
        QMessageBox::critical(this,
                              "Critical",
                              "Face Template Create Complete!",
                              QMessageBox::Yes,
                              QMessageBox::Yes);
        bfirst_ = true;
    }
    else
    {
        bfirst_ = false;
    }
    */
    // option-3: recognize faces
    struct face_descriptor *cur_face_info;
    if(faceDetection_.RecognizeFace(frame, frame_index_))
    {
        cur_face_info = faceDetection_.GetCurFaceInfo();
        // do process...
    }

    // update frame number:
    frame_index_++;

    // Render the frame
    ui->canvas->setPixmap(QPixmap::fromImage(OpenCVUtil::CVImgToQTImg(faceDetection_.DetectFace(frame, cv::Mat()))));
}

void MainWindow::on_menuFileExit_triggered()
{
    QWidget::close();
}

void MainWindow::on_pushButton_clicked()
{
    assert(dataContext_.GetMode() == RECORD);

    QFileDialog fd(this, QStringLiteral("选择录像文件"), ".", QStringLiteral("视频文件(*.avi)"));
    fd.setFileMode(QFileDialog::AnyFile);
    if(fd.exec() != QDialog::Accepted)
        return;

    ui->lineEditRecordPath->setText(fd.selectedFiles()[0]);
}

void MainWindow::on_pushButton_2_clicked()
{
    assert(dataContext_.GetMode() == RECORD);

    dataContext_.SetRecordStatus(!dataContext_.GetRecordStatus());

    if (dataContext_.GetRecordStatus())
    {
        assert(videoWriter_ == nullptr);
        videoWriter_ = new cv::VideoWriter();
        const string path = ui->lineEditRecordPath->text().toUtf8().constData();   // Form the new name with container

        cv::Size S = cv::Size(capture_.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                      capture_.get(CV_CAP_PROP_FRAME_HEIGHT));

        videoWriter_->open(path, -1/*capture_.get(CV_CAP_PROP_FOURCC)*/, capture_.get(CV_CAP_PROP_FPS), S, true);

        if (!videoWriter_->isOpened())
        {
            QMessageBox::critical(
                this,
                QStringLiteral("错误"),
                QStringLiteral("无法打开写入文件"),
                QMessageBox::Yes,
                QMessageBox::Yes);
            delete videoWriter_;
            videoWriter_ = nullptr;
            dataContext_.SetRecordStatus(!dataContext_.GetRecordStatus());
            return;
        }

        ui->pushButton_2->setText(QStringLiteral("停止录像"));
    }
    else
    {
        ui->pushButton_2->setText(QStringLiteral("开始录像"));

        assert(videoWriter_ != nullptr);
        delete videoWriter_;
        videoWriter_ = nullptr;
    }

}
