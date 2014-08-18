/********************************************************************************
** Form generated from reading UI file 'dialogvideosource.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGVIDEOSOURCE_H
#define UI_DIALOGVIDEOSOURCE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogVideoSource
{
public:
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QRadioButton *radioButtonCamera;
    QRadioButton *radioButtonFile;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEditVideoFile;
    QPushButton *pushButtonVideoFile;
    QDialogButtonBox *buttonBox;
    QButtonGroup *buttonGroupVideoSource;

    void setupUi(QDialog *DialogVideoSource)
    {
        if (DialogVideoSource->objectName().isEmpty())
            DialogVideoSource->setObjectName(QStringLiteral("DialogVideoSource"));
        DialogVideoSource->resize(525, 168);
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        DialogVideoSource->setFont(font);
        verticalLayout_4 = new QVBoxLayout(DialogVideoSource);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setSizeConstraint(QLayout::SetFixedSize);
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        groupBox = new QGroupBox(DialogVideoSource);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout_2 = new QHBoxLayout(groupBox);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        radioButtonCamera = new QRadioButton(groupBox);
        buttonGroupVideoSource = new QButtonGroup(DialogVideoSource);
        buttonGroupVideoSource->setObjectName(QStringLiteral("buttonGroupVideoSource"));
        buttonGroupVideoSource->addButton(radioButtonCamera);
        radioButtonCamera->setObjectName(QStringLiteral("radioButtonCamera"));
        radioButtonCamera->setChecked(true);

        verticalLayout->addWidget(radioButtonCamera);

        radioButtonFile = new QRadioButton(groupBox);
        buttonGroupVideoSource->addButton(radioButtonFile);
        radioButtonFile->setObjectName(QStringLiteral("radioButtonFile"));

        verticalLayout->addWidget(radioButtonFile);


        verticalLayout_2->addLayout(verticalLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        lineEditVideoFile = new QLineEdit(groupBox);
        lineEditVideoFile->setObjectName(QStringLiteral("lineEditVideoFile"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lineEditVideoFile->sizePolicy().hasHeightForWidth());
        lineEditVideoFile->setSizePolicy(sizePolicy);
        lineEditVideoFile->setMinimumSize(QSize(400, 0));

        horizontalLayout->addWidget(lineEditVideoFile);

        pushButtonVideoFile = new QPushButton(groupBox);
        pushButtonVideoFile->setObjectName(QStringLiteral("pushButtonVideoFile"));

        horizontalLayout->addWidget(pushButtonVideoFile);


        verticalLayout_2->addLayout(horizontalLayout);


        horizontalLayout_2->addLayout(verticalLayout_2);


        verticalLayout_3->addWidget(groupBox);

        buttonBox = new QDialogButtonBox(DialogVideoSource);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_3->addWidget(buttonBox);


        verticalLayout_4->addLayout(verticalLayout_3);


        retranslateUi(DialogVideoSource);
        QObject::connect(buttonBox, SIGNAL(accepted()), DialogVideoSource, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DialogVideoSource, SLOT(reject()));

        QMetaObject::connectSlotsByName(DialogVideoSource);
    } // setupUi

    void retranslateUi(QDialog *DialogVideoSource)
    {
        DialogVideoSource->setWindowTitle(QApplication::translate("DialogVideoSource", "\351\200\211\346\213\251\350\247\206\351\242\221\346\272\220", 0));
        groupBox->setTitle(QApplication::translate("DialogVideoSource", "\350\247\206\351\242\221\346\272\220", 0));
        radioButtonCamera->setText(QApplication::translate("DialogVideoSource", "\346\221\204\345\203\217\345\244\264", 0));
        radioButtonFile->setText(QApplication::translate("DialogVideoSource", "\350\247\206\351\242\221\346\226\207\344\273\266", 0));
        pushButtonVideoFile->setText(QApplication::translate("DialogVideoSource", "\351\200\211\346\213\251\346\226\207\344\273\266", 0));
    } // retranslateUi

};

namespace Ui {
    class DialogVideoSource: public Ui_DialogVideoSource {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGVIDEOSOURCE_H
