/****************************************************************************
** Meta object code from reading C++ file 'CanvasLabel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/ui/CanvasLabel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CanvasLabel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
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
struct qt_meta_tag_ZN11CanvasLabelE_t {};
} // unnamed namespace

template <> constexpr inline auto CanvasLabel::qt_create_metaobjectdata<qt_meta_tag_ZN11CanvasLabelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CanvasLabel",
        "sendMousePosition",
        "",
        "QPoint&",
        "pos",
        "Mouse_Pos",
        "leftReleased",
        "rightClicked",
        "panDelta",
        "dx",
        "dy",
        "dragScrolled",
        "wheelScrolled",
        "delta"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'sendMousePosition'
        QtMocHelpers::SignalData<void(QPoint &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'Mouse_Pos'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'leftReleased'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'rightClicked'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'panDelta'
        QtMocHelpers::SignalData<void(int, int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 }, { QMetaType::Int, 10 },
        }}),
        // Signal 'dragScrolled'
        QtMocHelpers::SignalData<void(int)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 10 },
        }}),
        // Signal 'wheelScrolled'
        QtMocHelpers::SignalData<void(int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CanvasLabel, qt_meta_tag_ZN11CanvasLabelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CanvasLabel::staticMetaObject = { {
    QMetaObject::SuperData::link<QLabel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CanvasLabelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CanvasLabelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11CanvasLabelE_t>.metaTypes,
    nullptr
} };

void CanvasLabel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CanvasLabel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sendMousePosition((*reinterpret_cast<std::add_pointer_t<QPoint&>>(_a[1]))); break;
        case 1: _t->Mouse_Pos(); break;
        case 2: _t->leftReleased(); break;
        case 3: _t->rightClicked(); break;
        case 4: _t->panDelta((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->dragScrolled((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->wheelScrolled((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CanvasLabel::*)(QPoint & )>(_a, &CanvasLabel::sendMousePosition, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CanvasLabel::*)()>(_a, &CanvasLabel::Mouse_Pos, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CanvasLabel::*)()>(_a, &CanvasLabel::leftReleased, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CanvasLabel::*)()>(_a, &CanvasLabel::rightClicked, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (CanvasLabel::*)(int , int )>(_a, &CanvasLabel::panDelta, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (CanvasLabel::*)(int )>(_a, &CanvasLabel::dragScrolled, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (CanvasLabel::*)(int )>(_a, &CanvasLabel::wheelScrolled, 6))
            return;
    }
}

const QMetaObject *CanvasLabel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CanvasLabel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CanvasLabelE_t>.strings))
        return static_cast<void*>(this);
    return QLabel::qt_metacast(_clname);
}

int CanvasLabel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLabel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void CanvasLabel::sendMousePosition(QPoint & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void CanvasLabel::Mouse_Pos()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CanvasLabel::leftReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CanvasLabel::rightClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void CanvasLabel::panDelta(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void CanvasLabel::dragScrolled(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void CanvasLabel::wheelScrolled(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}
QT_WARNING_POP
