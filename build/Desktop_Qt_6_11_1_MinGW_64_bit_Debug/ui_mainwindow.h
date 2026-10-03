/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "my_label.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainVerticalLayout;
    QFrame *topRibbon;
    QHBoxLayout *ribbonLayout;
    QLabel *lblTitle;
    QLabel *lblScore;
    QLabel *lblHighScore;
    QLabel *lblHearts;
    QLabel *lblStatus;
    QCheckBox *chkShowGrid;
    QLabel *lblScaleText;
    QSpinBox *spinScale;
    QPushButton *btnPause;
    QPushButton *btnRestart;
    my_label *frame;
    QLabel *lblHelp;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1024, 720);
        MainWindow->setMinimumSize(QSize(800, 600));
        MainWindow->setStyleSheet(QString::fromUtf8("\n"
"QMainWindow, QWidget#centralwidget {\n"
"    background-color: #12141c;\n"
"    color: #e0e6ed;\n"
"    font-family: 'Segoe UI', sans-serif;\n"
"}\n"
"QFrame#topRibbon {\n"
"    background-color: #1a1e2b;\n"
"    border-bottom: 2px solid #2a3144;\n"
"    padding: 6px 12px;\n"
"}\n"
"QLabel {\n"
"    color: #cfd6e0;\n"
"    font-size: 12px;\n"
"}\n"
"QLabel#lblTitle {\n"
"    color: #ffd166;\n"
"    font-size: 17px;\n"
"    font-weight: bold;\n"
"    letter-spacing: 1px;\n"
"}\n"
"QLabel#lblScore {\n"
"    color: #06d6a0;\n"
"    font-size: 15px;\n"
"    font-weight: bold;\n"
"    background-color: #102621;\n"
"    border: 1px solid #145946;\n"
"    border-radius: 4px;\n"
"    padding: 3px 8px;\n"
"}\n"
"QLabel#lblHighScore {\n"
"    color: #ffb703;\n"
"    font-size: 13px;\n"
"    font-weight: bold;\n"
"    background-color: #2b2310;\n"
"    border: 1px solid #5e4919;\n"
"    border-radius: 4px;\n"
"    padding: 3px 8px;\n"
"}\n"
"QLabel#lblHearts {\n"
"    color: #ef476f;\n"
"    font-size: 18px;\n"
"   "
                        " font-weight: bold;\n"
"    padding: 0 4px;\n"
"}\n"
"QLabel#lblStatus {\n"
"    color: #a8c7fa;\n"
"    font-size: 12px;\n"
"    font-weight: 500;\n"
"}\n"
"QLabel#lblHelp {\n"
"    background-color: #181c28;\n"
"    color: #8b9bb4;\n"
"    font-size: 11px;\n"
"    padding: 4px 10px;\n"
"    border-top: 1px solid #262c3e;\n"
"}\n"
"QPushButton {\n"
"    background-color: #252b3d;\n"
"    color: #e0e6ed;\n"
"    border: 1px solid #3d4763;\n"
"    border-radius: 4px;\n"
"    padding: 4px 12px;\n"
"    font-size: 12px;\n"
"    font-weight: 600;\n"
"    min-height: 24px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #323a52;\n"
"    border-color: #56648b;\n"
"}\n"
"QPushButton:pressed {\n"
"    background-color: #1b202e;\n"
"}\n"
"QPushButton#btnRestart {\n"
"    background-color: #1d3557;\n"
"    border-color: #457b9d;\n"
"    color: #f1faee;\n"
"}\n"
"QPushButton#btnRestart:hover {\n"
"    background-color: #274775;\n"
"    border-color: #a8dadc;\n"
"}\n"
"QSpinBox {\n"
"    background-color: #1a1e2b;"
                        "\n"
"    color: #e0e6ed;\n"
"    border: 1px solid #3d4763;\n"
"    border-radius: 4px;\n"
"    padding: 2px 6px;\n"
"    font-size: 12px;\n"
"    min-height: 24px;\n"
"}\n"
"QCheckBox {\n"
"    color: #cfd6e0;\n"
"    font-size: 12px;\n"
"    spacing: 5px;\n"
"}\n"
"   "));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainVerticalLayout = new QVBoxLayout(centralwidget);
        mainVerticalLayout->setSpacing(0);
        mainVerticalLayout->setObjectName("mainVerticalLayout");
        mainVerticalLayout->setContentsMargins(0, 0, 0, 0);
        topRibbon = new QFrame(centralwidget);
        topRibbon->setObjectName("topRibbon");
        topRibbon->setFrameShape(QFrame::Shape::NoFrame);
        ribbonLayout = new QHBoxLayout(topRibbon);
        ribbonLayout->setSpacing(12);
        ribbonLayout->setObjectName("ribbonLayout");
        ribbonLayout->setContentsMargins(12, 6, 12, 6);
        lblTitle = new QLabel(topRibbon);
        lblTitle->setObjectName("lblTitle");

        ribbonLayout->addWidget(lblTitle);

        lblScore = new QLabel(topRibbon);
        lblScore->setObjectName("lblScore");

        ribbonLayout->addWidget(lblScore);

        lblHighScore = new QLabel(topRibbon);
        lblHighScore->setObjectName("lblHighScore");

        ribbonLayout->addWidget(lblHighScore);

        lblHearts = new QLabel(topRibbon);
        lblHearts->setObjectName("lblHearts");

        ribbonLayout->addWidget(lblHearts);

        lblStatus = new QLabel(topRibbon);
        lblStatus->setObjectName("lblStatus");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lblStatus->sizePolicy().hasHeightForWidth());
        lblStatus->setSizePolicy(sizePolicy);

        ribbonLayout->addWidget(lblStatus);

        chkShowGrid = new QCheckBox(topRibbon);
        chkShowGrid->setObjectName("chkShowGrid");
        chkShowGrid->setChecked(true);

        ribbonLayout->addWidget(chkShowGrid);

        lblScaleText = new QLabel(topRibbon);
        lblScaleText->setObjectName("lblScaleText");

        ribbonLayout->addWidget(lblScaleText);

        spinScale = new QSpinBox(topRibbon);
        spinScale->setObjectName("spinScale");
        spinScale->setMinimum(2);
        spinScale->setMaximum(24);
        spinScale->setValue(6);
        spinScale->setDisplayIntegerBase(10);

        ribbonLayout->addWidget(spinScale);

        btnPause = new QPushButton(topRibbon);
        btnPause->setObjectName("btnPause");

        ribbonLayout->addWidget(btnPause);

        btnRestart = new QPushButton(topRibbon);
        btnRestart->setObjectName("btnRestart");

        ribbonLayout->addWidget(btnRestart);


        mainVerticalLayout->addWidget(topRibbon);

        frame = new my_label(centralwidget);
        frame->setObjectName("frame");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy1);
        frame->setCursor(QCursor(Qt::CursorShape::CrossCursor));
        frame->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

        mainVerticalLayout->addWidget(frame);

        lblHelp = new QLabel(centralwidget);
        lblHelp->setObjectName("lblHelp");
        lblHelp->setAlignment(Qt::AlignmentFlag::AlignCenter);

        mainVerticalLayout->addWidget(lblHelp);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Egg Catcher \342\200\224 Raster Grid Game", nullptr));
        lblTitle->setText(QCoreApplication::translate("MainWindow", "\360\237\245\232 EGGSCELLENT CATCH", nullptr));
        lblScore->setText(QCoreApplication::translate("MainWindow", "SCORE: 0", nullptr));
        lblHighScore->setText(QCoreApplication::translate("MainWindow", "BEST: 0", nullptr));
        lblHearts->setText(QCoreApplication::translate("MainWindow", "\342\235\244\342\235\244\342\235\244", nullptr));
        lblStatus->setText(QCoreApplication::translate("MainWindow", "Ready! Catch eggs, avoid bombs!", nullptr));
        chkShowGrid->setText(QCoreApplication::translate("MainWindow", "Grid Lines", nullptr));
        lblScaleText->setText(QCoreApplication::translate("MainWindow", "Scale:", nullptr));
        btnPause->setText(QCoreApplication::translate("MainWindow", "Pause", nullptr));
        btnRestart->setText(QCoreApplication::translate("MainWindow", "Restart (R)", nullptr));
        frame->setText(QString());
        lblHelp->setText(QCoreApplication::translate("MainWindow", "Controls: [A / D] or [\342\227\204 / \342\226\272] Move Basket | [Space] Pause/Resume | [R] Restart | \360\237\245\232 Regular Egg (+10) | \342\255\220 Golden Egg (+50) | \360\237\222\243 Bomb (Instant Game Over!) | 3 Misses = Game Over", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
