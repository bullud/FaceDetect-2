#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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

