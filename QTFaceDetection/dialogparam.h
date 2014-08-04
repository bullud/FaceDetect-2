#ifndef DIALOGPARAM_H
#define DIALOGPARAM_H

#include <QDialog>

namespace Ui {
class DialogParam;
}

class FaceDetection;

class DialogParam : public QDialog
{
    Q_OBJECT
public:
    DialogParam(QWidget *parent, FaceDetection *faceDetection);
    ~DialogParam();

private slots:
    void accept();

private:
    Ui::DialogParam *ui;
    FaceDetection *faceDetection_;
};

#endif // DIALOGPARAM_H
