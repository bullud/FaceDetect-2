/********************************************************************************
** Form generated from reading UI file 'dialogParam.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGPARAM_H
#define UI_DIALOGPARAM_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DialogParam
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *lineEditMaxFaces;
    QLabel *label_2;
    QLineEdit *lineEditTpCount;
    QLabel *label_3;
    QLineEdit *lineEditMinTempFaces;
    QLabel *label_4;
    QLineEdit *lineEditSimilarGate;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogParam)
    {
        if (DialogParam->objectName().isEmpty())
            DialogParam->setObjectName(QStringLiteral("DialogParam"));
        DialogParam->setWindowModality(Qt::WindowModal);
        DialogParam->resize(261, 157);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DialogParam->sizePolicy().hasHeightForWidth());
        DialogParam->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        DialogParam->setFont(font);
        DialogParam->setModal(true);
        layoutWidget = new QWidget(DialogParam);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(9, 9, 242, 140));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(layoutWidget);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lineEditMaxFaces = new QLineEdit(layoutWidget);
        lineEditMaxFaces->setObjectName(QStringLiteral("lineEditMaxFaces"));
        sizePolicy.setHeightForWidth(lineEditMaxFaces->sizePolicy().hasHeightForWidth());
        lineEditMaxFaces->setSizePolicy(sizePolicy);

        gridLayout->addWidget(lineEditMaxFaces, 0, 1, 1, 1);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        lineEditTpCount = new QLineEdit(layoutWidget);
        lineEditTpCount->setObjectName(QStringLiteral("lineEditTpCount"));
        sizePolicy.setHeightForWidth(lineEditTpCount->sizePolicy().hasHeightForWidth());
        lineEditTpCount->setSizePolicy(sizePolicy);

        gridLayout->addWidget(lineEditTpCount, 1, 1, 1, 1);

        label_3 = new QLabel(layoutWidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        lineEditMinTempFaces = new QLineEdit(layoutWidget);
        lineEditMinTempFaces->setObjectName(QStringLiteral("lineEditMinTempFaces"));
        sizePolicy.setHeightForWidth(lineEditMinTempFaces->sizePolicy().hasHeightForWidth());
        lineEditMinTempFaces->setSizePolicy(sizePolicy);

        gridLayout->addWidget(lineEditMinTempFaces, 2, 1, 1, 1);

        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        lineEditSimilarGate = new QLineEdit(layoutWidget);
        lineEditSimilarGate->setObjectName(QStringLiteral("lineEditSimilarGate"));
        sizePolicy.setHeightForWidth(lineEditSimilarGate->sizePolicy().hasHeightForWidth());
        lineEditSimilarGate->setSizePolicy(sizePolicy);

        gridLayout->addWidget(lineEditSimilarGate, 3, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        buttonBox = new QDialogButtonBox(layoutWidget);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DialogParam);
        QObject::connect(buttonBox, SIGNAL(accepted()), DialogParam, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DialogParam, SLOT(reject()));

        QMetaObject::connectSlotsByName(DialogParam);
    } // setupUi

    void retranslateUi(QDialog *DialogParam)
    {
        DialogParam->setWindowTitle(QApplication::translate("DialogParam", "\345\217\202\346\225\260\350\256\276\347\275\256", 0));
        label->setText(QApplication::translate("DialogParam", "\346\234\200\345\244\232\346\243\200\346\265\213\344\272\272\350\204\270\344\270\252\346\225\260\357\274\232", 0));
        label_2->setText(QApplication::translate("DialogParam", "\350\267\237\350\270\252\347\211\271\345\276\201\347\202\271\344\270\252\346\225\260\357\274\232", 0));
        label_3->setText(QApplication::translate("DialogParam", "\346\234\200\345\260\221\345\273\272\346\250\241\344\272\272\350\204\270\346\225\260\357\274\232", 0));
        label_4->setText(QApplication::translate("DialogParam", "\347\233\270\344\274\274\351\230\210\345\200\274\357\274\232", 0));
    } // retranslateUi

};

namespace Ui {
    class DialogParam: public Ui_DialogParam {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGPARAM_H
