/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../mainwindow.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "Mouse_Pressed",
        "",
        "showMousePosition",
        "QPoint&",
        "pos",
        "on_clear_clicked",
        "on_spinBox_valueChanged",
        "arg1",
        "on_pushButton_2_clicked",
        "redrawPixels",
        "on_draw_line_clicked",
        "on_draw_circle_clicked",
        "on_draw_ellipse_clicked",
        "on_close_polygon_clicked",
        "on_clear_polygon_clicked",
        "on_fill_shape_clicked",
        "onAnimationTick",
        "on_check_animate_toggled",
        "checked",
        "on_btn_draw_color_clicked",
        "on_btn_fill_color_clicked",
        "on_combo_transform_type_currentIndexChanged",
        "index",
        "on_btn_apply_transform_clicked",
        "on_btn_undo_transform_clicked",
        "on_btn_reset_transform_clicked",
        "on_spin_param1_valueChanged",
        "val",
        "on_spin_param2_valueChanged",
        "on_spin_pivot_x_valueChanged",
        "on_spin_pivot_y_valueChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'Mouse_Pressed'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showMousePosition'
        QtMocHelpers::SlotData<void(QPoint &)>(3, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'on_clear_clicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_spinBox_valueChanged'
        QtMocHelpers::SlotData<void(int)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'on_pushButton_2_clicked'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'redrawPixels'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_draw_line_clicked'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_draw_circle_clicked'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_draw_ellipse_clicked'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_close_polygon_clicked'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_clear_polygon_clicked'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_fill_shape_clicked'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAnimationTick'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_check_animate_toggled'
        QtMocHelpers::SlotData<void(bool)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 19 },
        }}),
        // Slot 'on_btn_draw_color_clicked'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_btn_fill_color_clicked'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_combo_transform_type_currentIndexChanged'
        QtMocHelpers::SlotData<void(int)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 23 },
        }}),
        // Slot 'on_btn_apply_transform_clicked'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_btn_undo_transform_clicked'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_btn_reset_transform_clicked'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_spin_param1_valueChanged'
        QtMocHelpers::SlotData<void(double)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 28 },
        }}),
        // Slot 'on_spin_param2_valueChanged'
        QtMocHelpers::SlotData<void(double)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 28 },
        }}),
        // Slot 'on_spin_pivot_x_valueChanged'
        QtMocHelpers::SlotData<void(double)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 28 },
        }}),
        // Slot 'on_spin_pivot_y_valueChanged'
        QtMocHelpers::SlotData<void(double)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 28 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->Mouse_Pressed(); break;
        case 1: _t->showMousePosition((*reinterpret_cast<std::add_pointer_t<QPoint&>>(_a[1]))); break;
        case 2: _t->on_clear_clicked(); break;
        case 3: _t->on_spinBox_valueChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->on_pushButton_2_clicked(); break;
        case 5: _t->redrawPixels(); break;
        case 6: _t->on_draw_line_clicked(); break;
        case 7: _t->on_draw_circle_clicked(); break;
        case 8: _t->on_draw_ellipse_clicked(); break;
        case 9: _t->on_close_polygon_clicked(); break;
        case 10: _t->on_clear_polygon_clicked(); break;
        case 11: _t->on_fill_shape_clicked(); break;
        case 12: _t->onAnimationTick(); break;
        case 13: _t->on_check_animate_toggled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->on_btn_draw_color_clicked(); break;
        case 15: _t->on_btn_fill_color_clicked(); break;
        case 16: _t->on_combo_transform_type_currentIndexChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->on_btn_apply_transform_clicked(); break;
        case 18: _t->on_btn_undo_transform_clicked(); break;
        case 19: _t->on_btn_reset_transform_clicked(); break;
        case 20: _t->on_spin_param1_valueChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 21: _t->on_spin_param2_valueChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 22: _t->on_spin_pivot_x_valueChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 23: _t->on_spin_pivot_y_valueChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 24;
    }
    return _id;
}
QT_WARNING_POP
