#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencvutil.h"
#include "dialogparam.h"
#include "dialogvideosource.h"
#include "dialogabout.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QShortcut>
#include <string>
#include <algorithm>
#include <cassert>

using std::string;
using std::for_each;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer_(this),
    capture_(new cv::VideoCapture(0)),
    actionGroup(new QActionGroup(this)),
    videoWriter_(nullptr),
    frame_index_(0),
    bredetect_(false),
    statusBarMessage(new QLabel())
{
    ui->setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    actionGroup->addAction(ui->faceTemplate);
    actionGroup->addAction(ui->faceRecognition);
    actionGroup->addAction(ui->videoRecord);

    if (!capture_->isOpened())
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

    frame_index_ = 0;
    cv::Size screen_size(capture_->get(CV_CAP_PROP_FRAME_WIDTH), capture_->get(CV_CAP_PROP_FRAME_HEIGHT));
    if(!faceDetection_.Initialize(screen_size, QString("./face_database")))
    {
        QMessageBox::critical(
            this,
            QStringLiteral("错误"),
            QStringLiteral("加载人脸模板失败"),
            QMessageBox::Yes,
            QMessageBox::Yes);
        return;
    }

    RefreshFaceTemplates();

    QShortcut* shortcut;
    shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), ui->listWidget);
    connect(shortcut, SIGNAL(activated()), this, SLOT(deleteItem()));

    shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), ui->listWidgetFaces);
    connect(shortcut, SIGNAL(activated()), this, SLOT(deleteItemFaces()));

    /*shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), ui->listWidgetTemplateFace);
    connect(shortcut, SIGNAL(activated()), this, SLOT(deleteItemTemplate()));*/

    ui->statusBar->addPermanentWidget(statusBarMessage);

    adjustSize();

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
    if (dataContext_.GetRecordStatus())
    {
        QMessageBox::critical(
            this,
            QStringLiteral("错误"),
            QStringLiteral("请先退出录像模式。"),
            QMessageBox::Yes,
            QMessageBox::Yes);
        return;
    }

    if (dataContext_.GetTemplateStatus())
    {
        QMessageBox::critical(
            this,
            QStringLiteral("错误"),
            QStringLiteral("请先停止建模。"),
            QMessageBox::Yes,
            QMessageBox::Yes);
        return;
    }

    if (action == ui->faceTemplate)
    {
        UseCamera();
        dataContext_.SetMode(TEMPLATE);
        ui->groupBoxVideoRecord->hide();
        ui->groupBoxFace->hide();
        ui->groupBoxFaces->show();
        ui->groupBoxFaceTemplateControl->show();
        ui->groupBoxTemplate->show();
        ui->actionVideoSource->setEnabled(false);
        statusBarMessage->setText(QStringLiteral("当前模式： 人脸建模"));
    }
    else if (action == ui->faceRecognition)
    {
        dataContext_.SetMode(DETECTION);
        ui->groupBoxFaces->hide();
        ui->groupBoxVideoRecord->hide();
        ui->groupBoxTemplate->hide();
        ui->groupBoxFaceTemplateControl->hide();
        ui->groupBoxFace->show();
        ui->actionVideoSource->setEnabled(true);
        QString msg = QStringLiteral("当前模式： 人脸识别");
        statusBarMessage->setText(msg);
    }
    else if (action == ui->videoRecord)
    {
        UseCamera();
        dataContext_.SetMode(RECORD);
        ui->groupBoxFaces->hide();
        ui->groupBoxFace->hide();
        ui->groupBoxTemplate->hide();
        ui->groupBoxFaceTemplateControl->hide();
        ui->groupBoxVideoRecord->show();
        ui->actionVideoSource->setEnabled(false);
        statusBarMessage->setText(QStringLiteral("当前模式： 视频录制"));
    }
    else
        assert(0);

    adjustSize();
}

void MainWindow::OnTimeout()
{
    if (!capture_->isOpened()) return;

    // Capture one frame from the camera
    cv::Mat frame;
    *capture_ >> frame;
    if(frame.empty())
        return;

    if (dataContext_.GetMode() == DETECTION)
    {
        if(faceDetection_.RecognizeFace(frame, frame_index_, bredetect_))
        {
            vector<struct recognized_info> recognized_faces;
            if(faceDetection_.QueryRecognizedFaces(recognized_faces))
            {
                for(size_t i=0; i<recognized_faces.size(); ++i)
                    OpenCVUtil::AddFaceItem(ui->listWidget, recognized_faces[i]._image, recognized_faces[i]._label);
            }
        }
        else
        {
            ui->listWidget->clear();
        }
        // we no need re-detect in default...
        bredetect_ = false;
    }
    else if (dataContext_.GetMode() == TEMPLATE)
    {
        //if (dataContext_.GetTemplateStatus())
        {
            // just detect/track face....
            faceDetection_.DetectFace(frame, frame_index_);
            /*
            {

                face_parameter param;
                faceDetection_.QueryParameters(&param);
                ui->progressBarFaceTemplate->setValue(createdTemplates * 100 / param._min_temp_faces);

                QMessageBox::critical(this,
                                      QStringLiteral("错误"),
                                      QStringLiteral("人脸模型创建成功!"),
                                      QMessageBox::Yes,
                                      QMessageBox::Yes);
                bfirst_ = true;

                dataContext_.SetTemplateStatus(false);

                if (dataContext_.GetTemplateStatus())
                {
                    ui->pushButtonStartStopTemplate->setText(QStringLiteral("停止建模"));
                }
                else
                {
                    ui->pushButtonStartStopTemplate->setText(QStringLiteral("开始建模"));
                    ui->progressBarFaceTemplate->hide();
                }
            }
            else
            {
                bfirst_ = false;
                face_parameter param;
                faceDetection_.QueryParameters(&param);
                ui->progressBarFaceTemplate->setValue(createdTemplates * 100 / param._min_temp_faces);
            }
            */
        }
    }
    else if (dataContext_.GetMode() == RECORD)
    {
        if (dataContext_.GetRecordStatus())
        {
            assert(videoWriter_ != nullptr);
            *videoWriter_ << frame;
        }
    }

    // update frame number:
    frame_index_++;

    // Render the frame
    ui->canvas->setPixmap(QPixmap::fromImage(OpenCVUtil::CVImgToQTImg(frame)));
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

        cv::Size S = cv::Size(capture_->get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                      capture_->get(CV_CAP_PROP_FRAME_HEIGHT));

        videoWriter_->open(path, -1/*capture_->get(CV_CAP_PROP_FOURCC)*/, capture_->get(CV_CAP_PROP_FPS), S, true);

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

void MainWindow::on_pushButtonStartStopTemplate_clicked()
{
    ui->listWidgetFaces->setEnabled(false);

    face_parameter param;
    faceDetection_.QueryParameters(&param);

    // the first face is unpressed face, when recognize one face, show this face to
    // tell user who is the recognized face, this face will also be save in face database...
    Mat face;
    if(faceTemplates_.empty())
    {
        // extract one unpressed face:
        if(faceDetection_.ExtractFace(face))
        {
            // 1. save it:
            faceTemplates_.push_back(face.clone());

            // 2. show it in list box:
            OpenCVUtil::AddFaceItem(ui->listWidgetTemplateFace, face, faceTemplates_.size());
        }
    }
    else
    {
        // get one normalized face:
        if((faceDetection_.CreateFaceTemplate(face)))
        {
            // 1. save it:
            faceTemplates_.push_back(face);

            // 2. display it in listbox:
            OpenCVUtil::AddFaceItem(ui->listWidgetTemplateFace, face, faceTemplates_.size());
        }
    }

    if (faceTemplates_.size() > param._min_temp_faces)
        ui->pushButtonSaveTemplate->setEnabled(true);
}

void MainWindow::on_actionSetParam_triggered()
{
    DialogParam dialog(this, &faceDetection_);
    dialog.exec();
}

void MainWindow::deleteItem()
{
    delete ui->listWidget->currentItem();
    bredetect_ = true;
}

void MainWindow::deleteItemTemplate()
{
}

void MainWindow::deleteItemFaces()
{
    int label = ui->listWidgetFaces->currentItem()->data(Qt::UserRole).toInt();
    if (label >= faceDetection_.GetFaceTemplateCount()) return;
    faceDetection_.DeleteFaceTemplates(label);
    RefreshFaceTemplates();
}

void MainWindow::on_actionVideoSource_triggered()
{
    DialogVideoSource dialog(this, capture_, dataContext_);
    dialog.exec();
}

void MainWindow::UseCamera()
{
    if (dataContext_.GetSource() == CAMERA) return;

    std::unique_ptr<cv::VideoCapture> newSource(new cv::VideoCapture(0));
    capture_.swap(newSource);
    dataContext_.SetSource(CAMERA);
}

void MainWindow::on_actionAbout_triggered()
{
    DialogAbout dialog(this);
    dialog.exec();
}

void MainWindow::on_pushButtonDeleteTemplate_clicked()
{
    ui->listWidgetFaces->setEnabled(false);

    if (faceTemplates_.size() == 0) return;
    faceTemplates_.pop_back();
    delete ui->listWidgetTemplateFace->item(faceTemplates_.size());

    face_parameter param;
    faceDetection_.QueryParameters(&param);
    if (faceTemplates_.size() <= param._min_temp_faces)
        ui->pushButtonSaveTemplate->setEnabled(false);
}

void MainWindow::on_pushButtonSaveTemplate_clicked()
{
    QListWidgetItem *current = ui->listWidgetFaces->currentItem();
    int label = current->data(Qt::UserRole).toInt();

    if (faceDetection_.GetFaceTemplateCount() == label)
    {
        // when face templates enough, do save action as below...
        if(!faceDetection_.SaveFaceTemplates(faceTemplates_))
        {
            QMessageBox::critical(
                this,
                QStringLiteral("错误"),
                QStringLiteral("写入模板文件失败"),
                QMessageBox::Yes,
                QMessageBox::Yes);
            return;
        }

        ui->listWidgetTemplateFace->clear();
        faceTemplates_.clear();
        QMessageBox::information(
            this,
            QStringLiteral("成功"),
            QStringLiteral("成功写入模板文件"),
            QMessageBox::Yes,
            QMessageBox::Yes);
    }
    else
    {
        if (!faceDetection_.SaveFaceTemplates(faceTemplates_, label))
        {
            QMessageBox::critical(
                this,
                QStringLiteral("错误"),
                QStringLiteral("写入模板文件失败"),
                QMessageBox::Yes,
                QMessageBox::Yes);
            return;
        }

        ui->listWidgetTemplateFace->clear();
        faceTemplates_.clear();
        QMessageBox::information(
            this,
            QStringLiteral("成功"),
            QStringLiteral("成功写入模板文件"),
            QMessageBox::Yes,
            QMessageBox::Yes);
    }

    ui->listWidgetFaces->setEnabled(true);
    ui->pushButtonSaveTemplate->setEnabled(false);

    RefreshFaceTemplates();
}

void MainWindow::on_listWidgetFaces_currentItemChanged(QListWidgetItem *current, QListWidgetItem * /*previous*/)
{
    if (nullptr == current) return;

    int label = current->data(Qt::UserRole).toInt();
    if (faceDetection_.GetFaceTemplateCount() == label)
    {
        ui->listWidgetTemplateFace->clear();
        faceTemplates_.clear();
    }
    else
    {
        ui->listWidgetTemplateFace->clear();
        faceTemplates_.clear();

        vector<Mat> templates { faceDetection_.GetFaceTemplates(label) };
        for_each(templates.begin(), templates.end(), [this](const Mat &face){
            faceTemplates_.push_back(face);
            OpenCVUtil::AddFaceItem(ui->listWidgetTemplateFace, face, faceTemplates_.size());
        });
    }
}

void MainWindow::RefreshFaceTemplates()
{
    ui->listWidgetFaces->clear();
    ui->listWidgetTemplateFace->clear();
    faceTemplates_.clear();

    for (int i = 0; i < faceDetection_.GetFaceTemplateCount(); ++i)
    {
        vector<Mat> templates { faceDetection_.GetFaceTemplates(i) };
        OpenCVUtil::AddFaceItem(ui->listWidgetFaces, *templates.begin(), i);
    }
    QListWidgetItem *item = new QListWidgetItem(QStringLiteral("新建"));
    item->setData(Qt::UserRole, faceDetection_.GetFaceTemplateCount());
    ui->listWidgetFaces->addItem(item);

    ui->listWidgetFaces->selectionModel()->reset();
}
