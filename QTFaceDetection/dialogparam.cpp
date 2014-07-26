#include "dialogparam.h"
#include "ui_dialogParam.h"
#include "facedetection.h"

DialogParam::DialogParam(QWidget *parent, FaceDetection *faceDetection) :
    QDialog(parent),
    ui(new Ui::DialogParam),
    faceDetection_(faceDetection)
{
    ui->setupUi(this);

    face_parameter param;
    faceDetection_->QueryParameters(&param);

    ui->lineEditMaxFaces->setText(QString::number(param._max_faces));
    ui->lineEditTpCount->setText(QString::number(param._min_kp_count));
    ui->lineEditMinTempFaces->setText(QString::number(param._min_temp_faces));
    ui->lineEditSimilarGate->setText(QString::number(param._similar_gate));
}

DialogParam::~DialogParam()
{
    delete ui;
}

void DialogParam::accept()
{
    face_parameter param = {
        ui->lineEditMaxFaces->text().toInt(),
        ui->lineEditTpCount->text().toInt(),
        ui->lineEditMinTempFaces->text().toInt(),
        ui->lineEditSimilarGate->text().toDouble()
    };
    faceDetection_->ModifyParameters(&param);
    QDialog::accept();
}
