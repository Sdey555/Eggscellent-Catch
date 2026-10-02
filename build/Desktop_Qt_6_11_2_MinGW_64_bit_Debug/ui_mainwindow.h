/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "my_label.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainVerticalLayout;
    QScrollArea *ribbonScrollArea;
    QWidget *ribbonContent;
    QHBoxLayout *ribbonLayout;
    QGroupBox *groupGrid;
    QGridLayout *gridGroupLayout;
    QLabel *label_scale;
    QSpinBox *spinBox;
    QPushButton *pushButton_2;
    QPushButton *clear;
    QGroupBox *groupAnimation;
    QGridLayout *animGroupLayout;
    QCheckBox *check_animate;
    QLabel *label_anim;
    QSpinBox *anim_delay;
    QLabel *label_batch;
    QSpinBox *anim_batch;
    QGroupBox *groupColors;
    QVBoxLayout *colorsGroupLayout;
    QPushButton *btn_draw_color;
    QPushButton *btn_fill_color;
    QGroupBox *groupLine;
    QVBoxLayout *lineGroupLayout;
    QComboBox *combo_line_algo;
    QPushButton *draw_line;
    QGroupBox *groupCircle;
    QGridLayout *circleGroupLayout;
    QLabel *label_radius;
    QSpinBox *circle_radius;
    QComboBox *combo_circle_algo;
    QPushButton *draw_circle;
    QGroupBox *groupEllipse;
    QGridLayout *ellipseGroupLayout;
    QLabel *label_rx;
    QSpinBox *ellipse_rx;
    QLabel *label_ry;
    QSpinBox *ellipse_ry;
    QComboBox *combo_ellipse_algo;
    QPushButton *draw_ellipse;
    QGroupBox *groupPolygon;
    QVBoxLayout *polygonGroupLayout;
    QPushButton *close_polygon;
    QPushButton *clear_polygon;
    QGroupBox *groupFill;
    QVBoxLayout *fillGroupLayout;
    QComboBox *combo_fill_algo;
    QPushButton *fill_shape;
    QGroupBox *groupTransform;
    QVBoxLayout *transformGroupLayout;
    QHBoxLayout *transformRowTop;
    QComboBox *combo_transform_type;
    QPushButton *btn_apply_transform;
    QPushButton *btn_undo_transform;
    QPushButton *btn_reset_transform;
    QHBoxLayout *transformRowBottom;
    QLabel *label_param1;
    QDoubleSpinBox *spin_param1;
    QLabel *label_param2;
    QDoubleSpinBox *spin_param2;
    QLabel *label_pivot_x;
    QDoubleSpinBox *spin_pivot_x;
    QLabel *label_pivot_y;
    QDoubleSpinBox *spin_pivot_y;
    QSpacerItem *ribbonSpacer;
    QHBoxLayout *workspaceLayout;
    QVBoxLayout *canvasLayout;
    QLabel *hint_label;
    my_label *frame;
    QScrollArea *statsScrollArea;
    QWidget *statsContent;
    QVBoxLayout *statsLayout;
    QGroupBox *groupCoords;
    QVBoxLayout *coordsLayout;
    QLabel *label_mouse_move;
    QLabel *mouse_movement;
    QLabel *label_mouse_press;
    QLabel *mouse_pressed;
    QGroupBox *groupMatrix;
    QVBoxLayout *matrixLayout;
    QLabel *label_matrix_display;
    QLabel *label_trans_status;
    QGroupBox *groupStatus;
    QVBoxLayout *statusSubLayout;
    QLabel *dda_time;
    QLabel *bressen_time;
    QLabel *polar_time;
    QLabel *cartesian_time;
    QLabel *midpoint_time;
    QLabel *ellipse_polar_time;
    QLabel *ellipse_midpoint_time;
    QLabel *fill_time;
    QLabel *total_pixels;
    QGroupBox *groupLegend;
    QVBoxLayout *legendLayout;
    QLabel *label_legend;
    QSpacerItem *statsVerticalSpacer;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1280, 800);
        MainWindow->setMinimumSize(QSize(1040, 680));
        MainWindow->setStyleSheet(QString::fromUtf8("\n"
"QMainWindow, QWidget#centralwidget { background-color: #202124; color: #e8eaed; font-family: 'Segoe UI', sans-serif; }\n"
"QScrollArea { border: none; background: transparent; }\n"
"QGroupBox {\n"
"  font-size: 11px; font-weight: 600; color: #e8eaed;\n"
"  border: 1px solid #3c4043; border-radius: 4px;\n"
"  margin-top: 14px; padding: 4px 5px 5px 5px;\n"
"}\n"
"QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 6px; padding: 0 3px; color: #a8c7fa; font-size: 10px; }\n"
"QPushButton {\n"
"  background-color: #35363a; color: #e8eaed;\n"
"  border: 1px solid #4a4d52; border-radius: 3px;\n"
"  padding: 2px 6px; font-size: 11px; min-height: 22px;\n"
"}\n"
"QPushButton:hover { background-color: #484a4e; border-color: #70757a; }\n"
"QPushButton:pressed { background-color: #292a2d; }\n"
"QPushButton#clear { background-color: #733333; border-color: #8c4040; }\n"
"QPushButton#clear:hover { background-color: #8c4040; }\n"
"QLabel { color: #cfd3d7; font-size: 11px; }\n"
"QLabel#hint_la"
                        "bel {\n"
"  color: #a8c7fa; background-color: #282a2d;\n"
"  border: 1px solid #3c4043; border-radius: 3px;\n"
"  padding: 4px 8px; font-size: 11px;\n"
"}\n"
"QSpinBox, QDoubleSpinBox {\n"
"  background-color: #292a2d; color: #e8eaed;\n"
"  border: 1px solid #4a4d52; border-radius: 3px;\n"
"  padding-left: 4px; padding-right: 18px;\n"
"  font-size: 11px; min-height: 22px;\n"
"}\n"
"QSpinBox::up-button, QDoubleSpinBox::up-button {\n"
"  subcontrol-origin: border;\n"
"  subcontrol-position: top right;\n"
"  width: 15px;\n"
"  border-left: 1px solid #4a4d52;\n"
"  border-bottom: 1px solid #3c4043;\n"
"  background-color: #35363a;\n"
"}\n"
"QSpinBox::down-button, QDoubleSpinBox::down-button {\n"
"  subcontrol-origin: border;\n"
"  subcontrol-position: bottom right;\n"
"  width: 15px;\n"
"  border-left: 1px solid #4a4d52;\n"
"  background-color: #35363a;\n"
"}\n"
"QSpinBox::up-button:hover, QSpinBox::down-button:hover,\n"
"QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover {\n"
"  background-color: "
                        "#484a4e;\n"
"}\n"
"QComboBox {\n"
"  background-color: #292a2d; color: #e8eaed;\n"
"  border: 1px solid #4a4d52; border-radius: 3px;\n"
"  padding-left: 5px; padding-right: 18px;\n"
"  font-size: 11px; min-height: 22px;\n"
"}\n"
"QComboBox::drop-down {\n"
"  subcontrol-origin: padding;\n"
"  subcontrol-position: top right;\n"
"  width: 15px;\n"
"  border-left: 1px solid #4a4d52;\n"
"  background-color: #35363a;\n"
"}\n"
"QComboBox QAbstractItemView {\n"
"  background-color: #292a2d; color: #e8eaed;\n"
"  selection-background-color: #3b5998; selection-color: #ffffff;\n"
"}\n"
"QCheckBox { color: #e8eaed; font-size: 11px; }\n"
"   "));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainVerticalLayout = new QVBoxLayout(centralwidget);
        mainVerticalLayout->setSpacing(6);
        mainVerticalLayout->setObjectName("mainVerticalLayout");
        mainVerticalLayout->setContentsMargins(8, 6, 8, 6);
        ribbonScrollArea = new QScrollArea(centralwidget);
        ribbonScrollArea->setObjectName("ribbonScrollArea");
        ribbonScrollArea->setMinimumSize(QSize(0, 104));
        ribbonScrollArea->setMaximumSize(QSize(16777215, 110));
        ribbonScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ribbonScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ribbonScrollArea->setWidgetResizable(true);
        ribbonScrollArea->setFrameShape(QFrame::NoFrame);
        ribbonContent = new QWidget();
        ribbonContent->setObjectName("ribbonContent");
        ribbonContent->setGeometry(QRect(0, 0, 1260, 102));
        ribbonLayout = new QHBoxLayout(ribbonContent);
        ribbonLayout->setSpacing(6);
        ribbonLayout->setObjectName("ribbonLayout");
        ribbonLayout->setContentsMargins(2, 0, 2, 2);
        groupGrid = new QGroupBox(ribbonContent);
        groupGrid->setObjectName("groupGrid");
        gridGroupLayout = new QGridLayout(groupGrid);
        gridGroupLayout->setSpacing(3);
        gridGroupLayout->setContentsMargins(3, 3, 3, 3);
        gridGroupLayout->setObjectName("gridGroupLayout");
        label_scale = new QLabel(groupGrid);
        label_scale->setObjectName("label_scale");

        gridGroupLayout->addWidget(label_scale, 0, 0, 1, 1);

        spinBox = new QSpinBox(groupGrid);
        spinBox->setObjectName("spinBox");
        spinBox->setMinimum(1);
        spinBox->setMaximum(50);
        spinBox->setValue(5);
        spinBox->setMinimumWidth(65);

        gridGroupLayout->addWidget(spinBox, 0, 1, 1, 1);

        pushButton_2 = new QPushButton(groupGrid);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setMinimumWidth(55);

        gridGroupLayout->addWidget(pushButton_2, 1, 0, 1, 1);

        clear = new QPushButton(groupGrid);
        clear->setObjectName("clear");
        clear->setMinimumWidth(55);

        gridGroupLayout->addWidget(clear, 1, 1, 1, 1);


        ribbonLayout->addWidget(groupGrid);

        groupAnimation = new QGroupBox(ribbonContent);
        groupAnimation->setObjectName("groupAnimation");
        animGroupLayout = new QGridLayout(groupAnimation);
        animGroupLayout->setSpacing(3);
        animGroupLayout->setContentsMargins(3, 3, 3, 3);
        animGroupLayout->setObjectName("animGroupLayout");
        check_animate = new QCheckBox(groupAnimation);
        check_animate->setObjectName("check_animate");

        animGroupLayout->addWidget(check_animate, 0, 0, 1, 1);

        label_anim = new QLabel(groupAnimation);
        label_anim->setObjectName("label_anim");

        animGroupLayout->addWidget(label_anim, 0, 1, 1, 1);

        anim_delay = new QSpinBox(groupAnimation);
        anim_delay->setObjectName("anim_delay");
        anim_delay->setMinimum(10);
        anim_delay->setMaximum(1000);
        anim_delay->setSingleStep(10);
        anim_delay->setValue(50);
        anim_delay->setMinimumWidth(72);

        animGroupLayout->addWidget(anim_delay, 0, 2, 1, 1);

        label_batch = new QLabel(groupAnimation);
        label_batch->setObjectName("label_batch");

        animGroupLayout->addWidget(label_batch, 1, 0, 1, 2);

        anim_batch = new QSpinBox(groupAnimation);
        anim_batch->setObjectName("anim_batch");
        anim_batch->setMinimum(1);
        anim_batch->setMaximum(200);
        anim_batch->setValue(12);
        anim_batch->setMinimumWidth(72);

        animGroupLayout->addWidget(anim_batch, 1, 2, 1, 1);


        ribbonLayout->addWidget(groupAnimation);

        groupColors = new QGroupBox(ribbonContent);
        groupColors->setObjectName("groupColors");
        colorsGroupLayout = new QVBoxLayout(groupColors);
        colorsGroupLayout->setSpacing(3);
        colorsGroupLayout->setContentsMargins(3, 3, 3, 3);
        colorsGroupLayout->setObjectName("colorsGroupLayout");
        btn_draw_color = new QPushButton(groupColors);
        btn_draw_color->setObjectName("btn_draw_color");
        btn_draw_color->setMinimumSize(QSize(90, 22));

        colorsGroupLayout->addWidget(btn_draw_color);

        btn_fill_color = new QPushButton(groupColors);
        btn_fill_color->setObjectName("btn_fill_color");
        btn_fill_color->setMinimumSize(QSize(90, 22));

        colorsGroupLayout->addWidget(btn_fill_color);


        ribbonLayout->addWidget(groupColors);

        groupLine = new QGroupBox(ribbonContent);
        groupLine->setObjectName("groupLine");
        lineGroupLayout = new QVBoxLayout(groupLine);
        lineGroupLayout->setSpacing(3);
        lineGroupLayout->setContentsMargins(3, 3, 3, 3);
        lineGroupLayout->setObjectName("lineGroupLayout");
        combo_line_algo = new QComboBox(groupLine);
        combo_line_algo->addItem(QString());
        combo_line_algo->addItem(QString());
        combo_line_algo->addItem(QString());
        combo_line_algo->setObjectName("combo_line_algo");
        combo_line_algo->setMinimumWidth(105);

        lineGroupLayout->addWidget(combo_line_algo);

        draw_line = new QPushButton(groupLine);
        draw_line->setObjectName("draw_line");
        draw_line->setMinimumWidth(105);

        lineGroupLayout->addWidget(draw_line);


        ribbonLayout->addWidget(groupLine);

        groupCircle = new QGroupBox(ribbonContent);
        groupCircle->setObjectName("groupCircle");
        circleGroupLayout = new QGridLayout(groupCircle);
        circleGroupLayout->setSpacing(3);
        circleGroupLayout->setContentsMargins(3, 3, 3, 3);
        circleGroupLayout->setObjectName("circleGroupLayout");
        label_radius = new QLabel(groupCircle);
        label_radius->setObjectName("label_radius");

        circleGroupLayout->addWidget(label_radius, 0, 0, 1, 1);

        circle_radius = new QSpinBox(groupCircle);
        circle_radius->setObjectName("circle_radius");
        circle_radius->setMinimum(1);
        circle_radius->setMaximum(250);
        circle_radius->setValue(20);
        circle_radius->setMinimumWidth(65);

        circleGroupLayout->addWidget(circle_radius, 0, 1, 1, 1);

        combo_circle_algo = new QComboBox(groupCircle);
        combo_circle_algo->addItem(QString());
        combo_circle_algo->addItem(QString());
        combo_circle_algo->addItem(QString());
        combo_circle_algo->addItem(QString());
        combo_circle_algo->setObjectName("combo_circle_algo");
        combo_circle_algo->setMinimumWidth(95);

        circleGroupLayout->addWidget(combo_circle_algo, 0, 2, 1, 1);

        draw_circle = new QPushButton(groupCircle);
        draw_circle->setObjectName("draw_circle");
        draw_circle->setMinimumWidth(175);

        circleGroupLayout->addWidget(draw_circle, 1, 0, 1, 3);


        ribbonLayout->addWidget(groupCircle);

        groupEllipse = new QGroupBox(ribbonContent);
        groupEllipse->setObjectName("groupEllipse");
        ellipseGroupLayout = new QGridLayout(groupEllipse);
        ellipseGroupLayout->setSpacing(3);
        ellipseGroupLayout->setContentsMargins(3, 3, 3, 3);
        ellipseGroupLayout->setObjectName("ellipseGroupLayout");
        label_rx = new QLabel(groupEllipse);
        label_rx->setObjectName("label_rx");

        ellipseGroupLayout->addWidget(label_rx, 0, 0, 1, 1);

        ellipse_rx = new QSpinBox(groupEllipse);
        ellipse_rx->setObjectName("ellipse_rx");
        ellipse_rx->setMinimum(1);
        ellipse_rx->setMaximum(250);
        ellipse_rx->setValue(28);
        ellipse_rx->setMinimumWidth(65);

        ellipseGroupLayout->addWidget(ellipse_rx, 0, 1, 1, 1);

        label_ry = new QLabel(groupEllipse);
        label_ry->setObjectName("label_ry");

        ellipseGroupLayout->addWidget(label_ry, 0, 2, 1, 1);

        ellipse_ry = new QSpinBox(groupEllipse);
        ellipse_ry->setObjectName("ellipse_ry");
        ellipse_ry->setMinimum(1);
        ellipse_ry->setMaximum(250);
        ellipse_ry->setValue(16);
        ellipse_ry->setMinimumWidth(65);

        ellipseGroupLayout->addWidget(ellipse_ry, 0, 3, 1, 1);

        combo_ellipse_algo = new QComboBox(groupEllipse);
        combo_ellipse_algo->addItem(QString());
        combo_ellipse_algo->addItem(QString());
        combo_ellipse_algo->addItem(QString());
        combo_ellipse_algo->setObjectName("combo_ellipse_algo");
        combo_ellipse_algo->setMinimumWidth(90);

        ellipseGroupLayout->addWidget(combo_ellipse_algo, 1, 0, 1, 2);

        draw_ellipse = new QPushButton(groupEllipse);
        draw_ellipse->setObjectName("draw_ellipse");
        draw_ellipse->setMinimumWidth(95);

        ellipseGroupLayout->addWidget(draw_ellipse, 1, 2, 1, 2);


        ribbonLayout->addWidget(groupEllipse);

        groupPolygon = new QGroupBox(ribbonContent);
        groupPolygon->setObjectName("groupPolygon");
        polygonGroupLayout = new QVBoxLayout(groupPolygon);
        polygonGroupLayout->setSpacing(3);
        polygonGroupLayout->setContentsMargins(3, 3, 3, 3);
        polygonGroupLayout->setObjectName("polygonGroupLayout");
        close_polygon = new QPushButton(groupPolygon);
        close_polygon->setObjectName("close_polygon");
        close_polygon->setMinimumWidth(85);

        polygonGroupLayout->addWidget(close_polygon);

        clear_polygon = new QPushButton(groupPolygon);
        clear_polygon->setObjectName("clear_polygon");
        clear_polygon->setMinimumWidth(85);

        polygonGroupLayout->addWidget(clear_polygon);


        ribbonLayout->addWidget(groupPolygon);

        groupFill = new QGroupBox(ribbonContent);
        groupFill->setObjectName("groupFill");
        fillGroupLayout = new QVBoxLayout(groupFill);
        fillGroupLayout->setSpacing(3);
        fillGroupLayout->setContentsMargins(3, 3, 3, 3);
        fillGroupLayout->setObjectName("fillGroupLayout");
        combo_fill_algo = new QComboBox(groupFill);
        combo_fill_algo->addItem(QString());
        combo_fill_algo->addItem(QString());
        combo_fill_algo->addItem(QString());
        combo_fill_algo->addItem(QString());
        combo_fill_algo->addItem(QString());
        combo_fill_algo->addItem(QString());
        combo_fill_algo->setObjectName("combo_fill_algo");
        combo_fill_algo->setMinimumWidth(175);

        fillGroupLayout->addWidget(combo_fill_algo);

        fill_shape = new QPushButton(groupFill);
        fill_shape->setObjectName("fill_shape");
        fill_shape->setMinimumWidth(175);

        fillGroupLayout->addWidget(fill_shape);


        ribbonLayout->addWidget(groupFill);

        groupTransform = new QGroupBox(ribbonContent);
        groupTransform->setObjectName("groupTransform");
        transformGroupLayout = new QVBoxLayout(groupTransform);
        transformGroupLayout->setSpacing(3);
        transformGroupLayout->setContentsMargins(3, 3, 3, 3);
        transformGroupLayout->setObjectName("transformGroupLayout");
        transformRowTop = new QHBoxLayout();
        transformRowTop->setSpacing(4);
        transformRowTop->setObjectName("transformRowTop");
        combo_transform_type = new QComboBox(groupTransform);
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->addItem(QString());
        combo_transform_type->setObjectName("combo_transform_type");
        combo_transform_type->setMinimumWidth(185);

        transformRowTop->addWidget(combo_transform_type);

        btn_apply_transform = new QPushButton(groupTransform);
        btn_apply_transform->setObjectName("btn_apply_transform");
        btn_apply_transform->setMinimumWidth(50);

        transformRowTop->addWidget(btn_apply_transform);

        btn_undo_transform = new QPushButton(groupTransform);
        btn_undo_transform->setObjectName("btn_undo_transform");
        btn_undo_transform->setMinimumWidth(48);

        transformRowTop->addWidget(btn_undo_transform);

        btn_reset_transform = new QPushButton(groupTransform);
        btn_reset_transform->setObjectName("btn_reset_transform");
        btn_reset_transform->setMinimumWidth(48);

        transformRowTop->addWidget(btn_reset_transform);


        transformGroupLayout->addLayout(transformRowTop);

        transformRowBottom = new QHBoxLayout();
        transformRowBottom->setSpacing(3);
        transformRowBottom->setObjectName("transformRowBottom");
        label_param1 = new QLabel(groupTransform);
        label_param1->setObjectName("label_param1");

        transformRowBottom->addWidget(label_param1);

        spin_param1 = new QDoubleSpinBox(groupTransform);
        spin_param1->setObjectName("spin_param1");
        spin_param1->setDecimals(1);
        spin_param1->setMinimum(-1000.000000000000000);
        spin_param1->setMaximum(1000.000000000000000);
        spin_param1->setValue(10.000000000000000);
        spin_param1->setSingleStep(1.000000000000000);
        spin_param1->setMinimumWidth(72);

        transformRowBottom->addWidget(spin_param1);

        label_param2 = new QLabel(groupTransform);
        label_param2->setObjectName("label_param2");

        transformRowBottom->addWidget(label_param2);

        spin_param2 = new QDoubleSpinBox(groupTransform);
        spin_param2->setObjectName("spin_param2");
        spin_param2->setDecimals(1);
        spin_param2->setMinimum(-1000.000000000000000);
        spin_param2->setMaximum(1000.000000000000000);
        spin_param2->setValue(10.000000000000000);
        spin_param2->setSingleStep(1.000000000000000);
        spin_param2->setMinimumWidth(72);

        transformRowBottom->addWidget(spin_param2);

        label_pivot_x = new QLabel(groupTransform);
        label_pivot_x->setObjectName("label_pivot_x");

        transformRowBottom->addWidget(label_pivot_x);

        spin_pivot_x = new QDoubleSpinBox(groupTransform);
        spin_pivot_x->setObjectName("spin_pivot_x");
        spin_pivot_x->setDecimals(1);
        spin_pivot_x->setMinimum(-1000.000000000000000);
        spin_pivot_x->setMaximum(1000.000000000000000);
        spin_pivot_x->setValue(0.000000000000000);
        spin_pivot_x->setSingleStep(1.000000000000000);
        spin_pivot_x->setMinimumWidth(65);

        transformRowBottom->addWidget(spin_pivot_x);

        label_pivot_y = new QLabel(groupTransform);
        label_pivot_y->setObjectName("label_pivot_y");

        transformRowBottom->addWidget(label_pivot_y);

        spin_pivot_y = new QDoubleSpinBox(groupTransform);
        spin_pivot_y->setObjectName("spin_pivot_y");
        spin_pivot_y->setDecimals(1);
        spin_pivot_y->setMinimum(-1000.000000000000000);
        spin_pivot_y->setMaximum(1000.000000000000000);
        spin_pivot_y->setValue(0.000000000000000);
        spin_pivot_y->setSingleStep(1.000000000000000);
        spin_pivot_y->setMinimumWidth(65);

        transformRowBottom->addWidget(spin_pivot_y);


        transformGroupLayout->addLayout(transformRowBottom);


        ribbonLayout->addWidget(groupTransform);

        ribbonSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ribbonLayout->addItem(ribbonSpacer);

        ribbonScrollArea->setWidget(ribbonContent);

        mainVerticalLayout->addWidget(ribbonScrollArea);

        workspaceLayout = new QHBoxLayout();
        workspaceLayout->setSpacing(6);
        workspaceLayout->setObjectName("workspaceLayout");
        canvasLayout = new QVBoxLayout();
        canvasLayout->setSpacing(4);
        canvasLayout->setObjectName("canvasLayout");
        hint_label = new QLabel(centralwidget);
        hint_label->setObjectName("hint_label");
        hint_label->setWordWrap(true);

        canvasLayout->addWidget(hint_label);

        frame = new my_label(centralwidget);
        frame->setObjectName("frame");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy);
        frame->setMinimumSize(QSize(500, 500));
        frame->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0); border: 1px solid #4a4d52; border-radius: 4px;"));
        frame->setAlignment(Qt::AlignCenter);

        canvasLayout->addWidget(frame);


        workspaceLayout->addLayout(canvasLayout);

        statsScrollArea = new QScrollArea(centralwidget);
        statsScrollArea->setObjectName("statsScrollArea");
        statsScrollArea->setMinimumSize(QSize(260, 0));
        statsScrollArea->setMaximumSize(QSize(290, 16777215));
        statsScrollArea->setWidgetResizable(true);
        statsScrollArea->setFrameShape(QFrame::NoFrame);
        statsContent = new QWidget();
        statsContent->setObjectName("statsContent");
        statsContent->setGeometry(QRect(0, 0, 270, 720));
        statsLayout = new QVBoxLayout(statsContent);
        statsLayout->setSpacing(6);
        statsLayout->setObjectName("statsLayout");
        statsLayout->setContentsMargins(4, 0, 4, 4);
        groupCoords = new QGroupBox(statsContent);
        groupCoords->setObjectName("groupCoords");
        coordsLayout = new QVBoxLayout(groupCoords);
        coordsLayout->setSpacing(3);
        coordsLayout->setObjectName("coordsLayout");
        label_mouse_move = new QLabel(groupCoords);
        label_mouse_move->setObjectName("label_mouse_move");

        coordsLayout->addWidget(label_mouse_move);

        mouse_movement = new QLabel(groupCoords);
        mouse_movement->setObjectName("mouse_movement");
        mouse_movement->setStyleSheet(QString::fromUtf8("font-family: monospace; font-weight: bold; color: #8ab4f8;"));

        coordsLayout->addWidget(mouse_movement);

        label_mouse_press = new QLabel(groupCoords);
        label_mouse_press->setObjectName("label_mouse_press");

        coordsLayout->addWidget(label_mouse_press);

        mouse_pressed = new QLabel(groupCoords);
        mouse_pressed->setObjectName("mouse_pressed");
        mouse_pressed->setStyleSheet(QString::fromUtf8("font-family: monospace; font-weight: bold; color: #f28b82;"));

        coordsLayout->addWidget(mouse_pressed);


        statsLayout->addWidget(groupCoords);

        groupMatrix = new QGroupBox(statsContent);
        groupMatrix->setObjectName("groupMatrix");
        matrixLayout = new QVBoxLayout(groupMatrix);
        matrixLayout->setSpacing(3);
        matrixLayout->setObjectName("matrixLayout");
        label_matrix_display = new QLabel(groupMatrix);
        label_matrix_display->setObjectName("label_matrix_display");
        label_matrix_display->setStyleSheet(QString::fromUtf8("font-family: Consolas, monospace; font-size: 10px; background-color: #1a1b1e; padding: 4px; border: 1px solid #3c4043; border-radius: 3px; color: #8ab4f8;"));

        matrixLayout->addWidget(label_matrix_display);

        label_trans_status = new QLabel(groupMatrix);
        label_trans_status->setObjectName("label_trans_status");
        label_trans_status->setWordWrap(true);
        label_trans_status->setStyleSheet(QString::fromUtf8("font-size: 10px; color: #cfd3d7;"));

        matrixLayout->addWidget(label_trans_status);


        statsLayout->addWidget(groupMatrix);

        groupStatus = new QGroupBox(statsContent);
        groupStatus->setObjectName("groupStatus");
        statusSubLayout = new QVBoxLayout(groupStatus);
        statusSubLayout->setSpacing(4);
        statusSubLayout->setObjectName("statusSubLayout");
        dda_time = new QLabel(groupStatus);
        dda_time->setObjectName("dda_time");
        dda_time->setWordWrap(true);

        statusSubLayout->addWidget(dda_time);

        bressen_time = new QLabel(groupStatus);
        bressen_time->setObjectName("bressen_time");
        bressen_time->setWordWrap(true);

        statusSubLayout->addWidget(bressen_time);

        polar_time = new QLabel(groupStatus);
        polar_time->setObjectName("polar_time");
        polar_time->setWordWrap(true);

        statusSubLayout->addWidget(polar_time);

        cartesian_time = new QLabel(groupStatus);
        cartesian_time->setObjectName("cartesian_time");
        cartesian_time->setWordWrap(true);

        statusSubLayout->addWidget(cartesian_time);

        midpoint_time = new QLabel(groupStatus);
        midpoint_time->setObjectName("midpoint_time");
        midpoint_time->setWordWrap(true);

        statusSubLayout->addWidget(midpoint_time);

        ellipse_polar_time = new QLabel(groupStatus);
        ellipse_polar_time->setObjectName("ellipse_polar_time");
        ellipse_polar_time->setWordWrap(true);

        statusSubLayout->addWidget(ellipse_polar_time);

        ellipse_midpoint_time = new QLabel(groupStatus);
        ellipse_midpoint_time->setObjectName("ellipse_midpoint_time");
        ellipse_midpoint_time->setWordWrap(true);

        statusSubLayout->addWidget(ellipse_midpoint_time);

        fill_time = new QLabel(groupStatus);
        fill_time->setObjectName("fill_time");
        fill_time->setWordWrap(true);

        statusSubLayout->addWidget(fill_time);

        total_pixels = new QLabel(groupStatus);
        total_pixels->setObjectName("total_pixels");
        total_pixels->setStyleSheet(QString::fromUtf8("font-weight: bold; color: #81c995;"));
        total_pixels->setWordWrap(true);

        statusSubLayout->addWidget(total_pixels);


        statsLayout->addWidget(groupStatus);

        groupLegend = new QGroupBox(statsContent);
        groupLegend->setObjectName("groupLegend");
        legendLayout = new QVBoxLayout(groupLegend);
        legendLayout->setObjectName("legendLayout");
        label_legend = new QLabel(groupLegend);
        label_legend->setObjectName("label_legend");
        label_legend->setWordWrap(true);
        label_legend->setStyleSheet(QString::fromUtf8("font-size: 10px; color: #9aa0a6;"));

        legendLayout->addWidget(label_legend);


        statsLayout->addWidget(groupLegend);

        statsVerticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        statsLayout->addItem(statsVerticalSpacer);

        statsScrollArea->setWidget(statsContent);

        workspaceLayout->addWidget(statsScrollArea);

        workspaceLayout->setStretch(0, 1);

        mainVerticalLayout->addLayout(workspaceLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1280, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Raster Drawing \342\200\224 Line, Circle, Ellipse, Fill, 2D Transformations", nullptr));
        groupGrid->setTitle(QCoreApplication::translate("MainWindow", "Grid", nullptr));
        label_scale->setText(QCoreApplication::translate("MainWindow", "Scale:", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "Redraw", nullptr));
        clear->setText(QCoreApplication::translate("MainWindow", "Clear", nullptr));
        groupAnimation->setTitle(QCoreApplication::translate("MainWindow", "Animation", nullptr));
        check_animate->setText(QCoreApplication::translate("MainWindow", "On", nullptr));
        label_anim->setText(QCoreApplication::translate("MainWindow", "Delay:", nullptr));
        label_batch->setText(QCoreApplication::translate("MainWindow", "Cells/step:", nullptr));
        groupColors->setTitle(QCoreApplication::translate("MainWindow", "Colors", nullptr));
        btn_draw_color->setText(QCoreApplication::translate("MainWindow", "Draw Color", nullptr));
        btn_fill_color->setText(QCoreApplication::translate("MainWindow", "Fill Color", nullptr));
        groupLine->setTitle(QCoreApplication::translate("MainWindow", "Line", nullptr));
        combo_line_algo->setItemText(0, QCoreApplication::translate("MainWindow", "DDA", nullptr));
        combo_line_algo->setItemText(1, QCoreApplication::translate("MainWindow", "Bresenham", nullptr));
        combo_line_algo->setItemText(2, QCoreApplication::translate("MainWindow", "Compare", nullptr));

        draw_line->setText(QCoreApplication::translate("MainWindow", "Draw Line", nullptr));
        groupCircle->setTitle(QCoreApplication::translate("MainWindow", "Circle", nullptr));
        label_radius->setText(QCoreApplication::translate("MainWindow", "r:", nullptr));
        combo_circle_algo->setItemText(0, QCoreApplication::translate("MainWindow", "Polar", nullptr));
        combo_circle_algo->setItemText(1, QCoreApplication::translate("MainWindow", "Cartesian", nullptr));
        combo_circle_algo->setItemText(2, QCoreApplication::translate("MainWindow", "Midpoint", nullptr));
        combo_circle_algo->setItemText(3, QCoreApplication::translate("MainWindow", "Compare", nullptr));

        draw_circle->setText(QCoreApplication::translate("MainWindow", "Draw Circle", nullptr));
        groupEllipse->setTitle(QCoreApplication::translate("MainWindow", "Ellipse", nullptr));
        label_rx->setText(QCoreApplication::translate("MainWindow", "rx:", nullptr));
        label_ry->setText(QCoreApplication::translate("MainWindow", "ry:", nullptr));
        combo_ellipse_algo->setItemText(0, QCoreApplication::translate("MainWindow", "Polar", nullptr));
        combo_ellipse_algo->setItemText(1, QCoreApplication::translate("MainWindow", "Midpoint", nullptr));
        combo_ellipse_algo->setItemText(2, QCoreApplication::translate("MainWindow", "Compare", nullptr));

        draw_ellipse->setText(QCoreApplication::translate("MainWindow", "Draw Ellipse", nullptr));
        groupPolygon->setTitle(QCoreApplication::translate("MainWindow", "Polygon", nullptr));
        close_polygon->setText(QCoreApplication::translate("MainWindow", "Close Poly", nullptr));
        clear_polygon->setText(QCoreApplication::translate("MainWindow", "Clear Poly", nullptr));
        groupFill->setTitle(QCoreApplication::translate("MainWindow", "Fill", nullptr));
        combo_fill_algo->setItemText(0, QCoreApplication::translate("MainWindow", "Boundary fill 4-conn", nullptr));
        combo_fill_algo->setItemText(1, QCoreApplication::translate("MainWindow", "Boundary fill 8-conn", nullptr));
        combo_fill_algo->setItemText(2, QCoreApplication::translate("MainWindow", "Flood fill 4-conn", nullptr));
        combo_fill_algo->setItemText(3, QCoreApplication::translate("MainWindow", "Flood fill 8-conn", nullptr));
        combo_fill_algo->setItemText(4, QCoreApplication::translate("MainWindow", "Scanline even-odd", nullptr));
        combo_fill_algo->setItemText(5, QCoreApplication::translate("MainWindow", "Scanline nonzero", nullptr));

        fill_shape->setText(QCoreApplication::translate("MainWindow", "Fill Shape", nullptr));
        groupTransform->setTitle(QCoreApplication::translate("MainWindow", "Transform (Polygon)", nullptr));
        combo_transform_type->setItemText(0, QCoreApplication::translate("MainWindow", "a) Translation (tx, ty)", nullptr));
        combo_transform_type->setItemText(1, QCoreApplication::translate("MainWindow", "b) Rotation (\316\270\302\260 Origin)", nullptr));
        combo_transform_type->setItemText(2, QCoreApplication::translate("MainWindow", "c) Scaling (sx, sy Origin)", nullptr));
        combo_transform_type->setItemText(3, QCoreApplication::translate("MainWindow", "d) Shear (shx, shy)", nullptr));
        combo_transform_type->setItemText(4, QCoreApplication::translate("MainWindow", "e) Reflection (X-Axis)", nullptr));
        combo_transform_type->setItemText(5, QCoreApplication::translate("MainWindow", "e) Reflection (Y-Axis)", nullptr));
        combo_transform_type->setItemText(6, QCoreApplication::translate("MainWindow", "e) Reflection (Origin)", nullptr));
        combo_transform_type->setItemText(7, QCoreApplication::translate("MainWindow", "f) Reflection (Line: y=mx+c)", nullptr));
        combo_transform_type->setItemText(8, QCoreApplication::translate("MainWindow", "f) Reflection (Line: 2 Points)", nullptr));
        combo_transform_type->setItemText(9, QCoreApplication::translate("MainWindow", "g) Rotation (Arbitrary Pt: px, py)", nullptr));
        combo_transform_type->setItemText(10, QCoreApplication::translate("MainWindow", "h) Scaling (Arbitrary Pt: px, py)", nullptr));

        btn_apply_transform->setText(QCoreApplication::translate("MainWindow", "Apply", nullptr));
        btn_undo_transform->setText(QCoreApplication::translate("MainWindow", "Undo", nullptr));
        btn_reset_transform->setText(QCoreApplication::translate("MainWindow", "Reset", nullptr));
        label_param1->setText(QCoreApplication::translate("MainWindow", "tx:", nullptr));
        label_param2->setText(QCoreApplication::translate("MainWindow", "ty:", nullptr));
        label_pivot_x->setText(QCoreApplication::translate("MainWindow", "px:", nullptr));
        label_pivot_y->setText(QCoreApplication::translate("MainWindow", "py:", nullptr));
        hint_label->setText(QCoreApplication::translate("MainWindow", "Click grid to choose points. For polygons: add vertices, close, then choose a seed or transform.", nullptr));
        frame->setText(QString());
        groupCoords->setTitle(QCoreApplication::translate("MainWindow", "Coordinates", nullptr));
        label_mouse_move->setText(QCoreApplication::translate("MainWindow", "Mouse Cursor:", nullptr));
        mouse_movement->setText(QCoreApplication::translate("MainWindow", "X : \342\200\224, Y : \342\200\224", nullptr));
        label_mouse_press->setText(QCoreApplication::translate("MainWindow", "Last Clicked / Centre:", nullptr));
        mouse_pressed->setText(QCoreApplication::translate("MainWindow", "X : \342\200\224, Y : \342\200\224", nullptr));
        groupMatrix->setTitle(QCoreApplication::translate("MainWindow", "Transform Matrix (3x3)", nullptr));
        label_matrix_display->setText(QCoreApplication::translate("MainWindow", "[ 1.00   0.00   0.00 ]\n"
"[ 0.00   1.00   0.00 ]\n"
"[ 0.00   0.00   1.00 ]", nullptr));
        label_trans_status->setText(QCoreApplication::translate("MainWindow", "Identity (No transform applied)", nullptr));
        groupStatus->setTitle(QCoreApplication::translate("MainWindow", "Statistics & Timings", nullptr));
        dda_time->setText(QCoreApplication::translate("MainWindow", "DDA: \342\200\224", nullptr));
        bressen_time->setText(QCoreApplication::translate("MainWindow", "Bresenham: \342\200\224", nullptr));
        polar_time->setText(QCoreApplication::translate("MainWindow", "Polar Circle: \342\200\224", nullptr));
        cartesian_time->setText(QCoreApplication::translate("MainWindow", "Cartesian Circle: \342\200\224", nullptr));
        midpoint_time->setText(QCoreApplication::translate("MainWindow", "Midpoint Circle: \342\200\224", nullptr));
        ellipse_polar_time->setText(QCoreApplication::translate("MainWindow", "Ellipse Polar: \342\200\224", nullptr));
        ellipse_midpoint_time->setText(QCoreApplication::translate("MainWindow", "Ellipse Midpoint: \342\200\224", nullptr));
        fill_time->setText(QCoreApplication::translate("MainWindow", "Fill: \342\200\224", nullptr));
        total_pixels->setText(QCoreApplication::translate("MainWindow", "Total Drawn Pixels: 0", nullptr));
        groupLegend->setTitle(QCoreApplication::translate("MainWindow", "Color Legend", nullptr));
        label_legend->setText(QCoreApplication::translate("MainWindow", "\342\200\242 Orange: Clicked point / seed\n"
"\342\200\242 Red: DDA\n"
"\342\200\242 Blue: Bresenham line\n"
"\342\200\242 Cyan: Polar circle / ellipse\n"
"\342\200\242 Yellow: Cartesian circle\n"
"\342\200\242 Green: Midpoint circle / ellipse\n"
"\342\200\242 Magenta: Overlap (2 methods)\n"
"\342\200\242 White: Overlap (3 methods)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
