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

private:
    Ui::DialogParam *ui;
    FaceDetection *faceDetection_;

public slots:
    void accept();
};

#endif // DIALOGPARAM_H
