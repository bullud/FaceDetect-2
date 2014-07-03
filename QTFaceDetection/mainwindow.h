#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "datacontext.h"
#include <QMainWindow>
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

private slots:
    void OnToggleMode();
};

#endif // MAINWINDOW_H
