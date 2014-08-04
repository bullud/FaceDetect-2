/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *menuFileExit;
    QAction *action_2;
    QAction *actionSetParam;
    QAction *action_4;
    QAction *faceTemplate;
    QAction *faceRecognition;
    QAction *videoRecord;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *canvas;
    QGroupBox *groupBoxFaceTempalte;
    QHBoxLayout *horizontalLayout_4;
    QListWidget *listWidget;
    QGroupBox *groupBoxVideoRecord;
    QHBoxLayout *horizontalLayout_5;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *lineEditRecordPath;
    QPushButton *pushButton;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_2;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *groupBoxFaceTemplateControl;
    QHBoxLayout *horizontalLayout_7;
    QVBoxLayout *verticalLayout_4;
    QProgressBar *progressBarFaceTemplate;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *pushButtonStartStopTemplate;
    QSpacerItem *horizontalSpacer_4;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuSettings;
    QMenu *menuHelp;
    QMenu *menuFeatures;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(806, 725);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        MainWindow->setFont(font);
        menuFileExit = new QAction(MainWindow);
        menuFileExit->setObjectName(QStringLiteral("menuFileExit"));
        menuFileExit->setFont(font);
        action_2 = new QAction(MainWindow);
        action_2->setObjectName(QStringLiteral("action_2"));
        action_2->setFont(font);
        actionSetParam = new QAction(MainWindow);
        actionSetParam->setObjectName(QStringLiteral("actionSetParam"));
        actionSetParam->setFont(font);
        action_4 = new QAction(MainWindow);
        action_4->setObjectName(QStringLiteral("action_4"));
        action_4->setFont(font);
        faceTemplate = new QAction(MainWindow);
        faceTemplate->setObjectName(QStringLiteral("faceTemplate"));
        faceTemplate->setCheckable(true);
        faceTemplate->setFont(font);
        faceRecognition = new QAction(MainWindow);
        faceRecognition->setObjectName(QStringLiteral("faceRecognition"));
        faceRecognition->setCheckable(true);
        faceRecognition->setFont(font);
        videoRecord = new QAction(MainWindow);
        videoRecord->setObjectName(QStringLiteral("videoRecord"));
        videoRecord->setCheckable(true);
        videoRecord->setFont(font);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy1);
        centralWidget->setFont(font);
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        canvas = new QLabel(centralWidget);
        canvas->setObjectName(QStringLiteral("canvas"));
        sizePolicy.setHeightForWidth(canvas->sizePolicy().hasHeightForWidth());
        canvas->setSizePolicy(sizePolicy);
        canvas->setMinimumSize(QSize(640, 480));
        canvas->setScaledContents(false);
        canvas->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(canvas);

        groupBoxFaceTempalte = new QGroupBox(centralWidget);
        groupBoxFaceTempalte->setObjectName(QStringLiteral("groupBoxFaceTempalte"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupBoxFaceTempalte->sizePolicy().hasHeightForWidth());
        groupBoxFaceTempalte->setSizePolicy(sizePolicy2);
        horizontalLayout_4 = new QHBoxLayout(groupBoxFaceTempalte);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        listWidget = new QListWidget(groupBoxFaceTempalte);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        sizePolicy2.setHeightForWidth(listWidget->sizePolicy().hasHeightForWidth());
        listWidget->setSizePolicy(sizePolicy2);
        listWidget->setMaximumSize(QSize(120, 16777215));
        listWidget->setIconSize(QSize(100, 100));
        listWidget->setResizeMode(QListView::Adjust);
        listWidget->setViewMode(QListView::IconMode);

        horizontalLayout_4->addWidget(listWidget);


        horizontalLayout->addWidget(groupBoxFaceTempalte);


        verticalLayout_2->addLayout(horizontalLayout);

        groupBoxVideoRecord = new QGroupBox(centralWidget);
        groupBoxVideoRecord->setObjectName(QStringLiteral("groupBoxVideoRecord"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(groupBoxVideoRecord->sizePolicy().hasHeightForWidth());
        groupBoxVideoRecord->setSizePolicy(sizePolicy3);
        horizontalLayout_5 = new QHBoxLayout(groupBoxVideoRecord);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label = new QLabel(groupBoxVideoRecord);
        label->setObjectName(QStringLiteral("label"));
        sizePolicy3.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy3);

        horizontalLayout_2->addWidget(label);

        lineEditRecordPath = new QLineEdit(groupBoxVideoRecord);
        lineEditRecordPath->setObjectName(QStringLiteral("lineEditRecordPath"));

        horizontalLayout_2->addWidget(lineEditRecordPath);

        pushButton = new QPushButton(groupBoxVideoRecord);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout_2->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        pushButton_2 = new QPushButton(groupBoxVideoRecord);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));

        horizontalLayout_3->addWidget(pushButton_2);

        horizontalSpacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_3);


        horizontalLayout_5->addLayout(verticalLayout);


        verticalLayout_2->addWidget(groupBoxVideoRecord);

        groupBoxFaceTemplateControl = new QGroupBox(centralWidget);
        groupBoxFaceTemplateControl->setObjectName(QStringLiteral("groupBoxFaceTemplateControl"));
        sizePolicy3.setHeightForWidth(groupBoxFaceTemplateControl->sizePolicy().hasHeightForWidth());
        groupBoxFaceTemplateControl->setSizePolicy(sizePolicy3);
        horizontalLayout_7 = new QHBoxLayout(groupBoxFaceTemplateControl);
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        progressBarFaceTemplate = new QProgressBar(groupBoxFaceTemplateControl);
        progressBarFaceTemplate->setObjectName(QStringLiteral("progressBarFaceTemplate"));
        sizePolicy3.setHeightForWidth(progressBarFaceTemplate->sizePolicy().hasHeightForWidth());
        progressBarFaceTemplate->setSizePolicy(sizePolicy3);
        progressBarFaceTemplate->setValue(0);

        verticalLayout_4->addWidget(progressBarFaceTemplate);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        horizontalSpacer_3 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_3);

        pushButtonStartStopTemplate = new QPushButton(groupBoxFaceTemplateControl);
        pushButtonStartStopTemplate->setObjectName(QStringLiteral("pushButtonStartStopTemplate"));

        horizontalLayout_6->addWidget(pushButtonStartStopTemplate);

        horizontalSpacer_4 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_4);


        verticalLayout_4->addLayout(horizontalLayout_6);


        horizontalLayout_7->addLayout(verticalLayout_4);


        verticalLayout_2->addWidget(groupBoxFaceTemplateControl);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 806, 23));
        menuBar->setFont(font);
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuSettings = new QMenu(menuBar);
        menuSettings->setObjectName(QStringLiteral("menuSettings"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        menuFeatures = new QMenu(menuBar);
        menuFeatures->setObjectName(QStringLiteral("menuFeatures"));
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuFeatures->menuAction());
        menuBar->addAction(menuSettings->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(menuFileExit);
        menuSettings->addAction(action_2);
        menuSettings->addAction(actionSetParam);
        menuHelp->addAction(action_4);
        menuFeatures->addAction(faceTemplate);
        menuFeatures->addAction(faceRecognition);
        menuFeatures->addAction(videoRecord);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "\344\272\272\350\204\270\350\257\206\345\210\253\347\263\273\347\273\237\346\274\224\347\244\272\347\250\213\345\272\217", 0));
        menuFileExit->setText(QApplication::translate("MainWindow", "\351\200\200\345\207\272", 0));
        action_2->setText(QApplication::translate("MainWindow", "\350\247\206\351\242\221\346\272\220", 0));
        actionSetParam->setText(QApplication::translate("MainWindow", "\345\217\202\346\225\260", 0));
        action_4->setText(QApplication::translate("MainWindow", "\345\205\263\344\272\216", 0));
        faceTemplate->setText(QApplication::translate("MainWindow", "\344\272\272\350\204\270\345\273\272\346\250\241", 0));
        faceRecognition->setText(QApplication::translate("MainWindow", "\344\272\272\350\204\270\350\257\206\345\210\253", 0));
        videoRecord->setText(QApplication::translate("MainWindow", "\350\247\206\351\242\221\345\275\225\345\210\266", 0));
        canvas->setText(QString());
        groupBoxFaceTempalte->setTitle(QApplication::translate("MainWindow", "\344\272\272\350\204\270\346\250\241\345\236\213", 0));
        groupBoxVideoRecord->setTitle(QApplication::translate("MainWindow", "\350\247\206\351\242\221\345\275\225\345\210\266", 0));
        label->setText(QApplication::translate("MainWindow", "\346\226\207\344\273\266\350\267\257\345\276\204\357\274\232", 0));
        pushButton->setText(QApplication::translate("MainWindow", "\346\226\207\344\273\266...", 0));
        pushButton_2->setText(QString());
        groupBoxFaceTemplateControl->setTitle(QApplication::translate("MainWindow", "\344\272\272\350\204\270\345\273\272\346\250\241", 0));
        pushButtonStartStopTemplate->setText(QString());
        menuFile->setTitle(QApplication::translate("MainWindow", "\346\226\207\344\273\266", 0));
        menuSettings->setTitle(QApplication::translate("MainWindow", "\350\256\276\347\275\256", 0));
        menuHelp->setTitle(QApplication::translate("MainWindow", "\345\270\256\345\212\251", 0));
        menuFeatures->setTitle(QApplication::translate("MainWindow", "\345\212\237\350\203\275", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
