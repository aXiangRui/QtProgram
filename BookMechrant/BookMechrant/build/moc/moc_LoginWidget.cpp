/****************************************************************************
** Meta object code from reading C++ file 'LoginWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../LoginWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LoginWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LoginWidget_t {
    QByteArrayData data[23];
    char stringdata0[295];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LoginWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LoginWidget_t qt_meta_stringdata_LoginWidget = {
    {
QT_MOC_LITERAL(0, 0, 11), // "LoginWidget"
QT_MOC_LITERAL(1, 12, 14), // "loginRequested"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 8), // "username"
QT_MOC_LITERAL(4, 37, 8), // "password"
QT_MOC_LITERAL(5, 46, 12), // "loginSuccess"
QT_MOC_LITERAL(6, 59, 17), // "registerRequested"
QT_MOC_LITERAL(7, 77, 14), // "forgotPassword"
QT_MOC_LITERAL(8, 92, 14), // "onLoginClicked"
QT_MOC_LITERAL(9, 107, 14), // "onLoginSuccess"
QT_MOC_LITERAL(10, 122, 13), // "onLoginFailed"
QT_MOC_LITERAL(11, 136, 5), // "error"
QT_MOC_LITERAL(12, 142, 17), // "onRememberChanged"
QT_MOC_LITERAL(13, 160, 5), // "state"
QT_MOC_LITERAL(14, 166, 15), // "onServerChanged"
QT_MOC_LITERAL(15, 182, 5), // "index"
QT_MOC_LITERAL(16, 188, 17), // "onRegisterClicked"
QT_MOC_LITERAL(17, 206, 23), // "onForgotPasswordClicked"
QT_MOC_LITERAL(18, 230, 18), // "onEyeButtonClicked"
QT_MOC_LITERAL(19, 249, 11), // "shakeWindow"
QT_MOC_LITERAL(20, 261, 18), // "showWelcomeMessage"
QT_MOC_LITERAL(21, 280, 6), // "fadeIn"
QT_MOC_LITERAL(22, 287, 7) // "opacity"

    },
    "LoginWidget\0loginRequested\0\0username\0"
    "password\0loginSuccess\0registerRequested\0"
    "forgotPassword\0onLoginClicked\0"
    "onLoginSuccess\0onLoginFailed\0error\0"
    "onRememberChanged\0state\0onServerChanged\0"
    "index\0onRegisterClicked\0onForgotPasswordClicked\0"
    "onEyeButtonClicked\0shakeWindow\0"
    "showWelcomeMessage\0fadeIn\0opacity"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LoginWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       1,  114, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   89,    2, 0x06 /* Public */,
       5,    0,   94,    2, 0x06 /* Public */,
       6,    0,   95,    2, 0x06 /* Public */,
       7,    0,   96,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   97,    2, 0x08 /* Private */,
       9,    0,   98,    2, 0x08 /* Private */,
      10,    1,   99,    2, 0x08 /* Private */,
      12,    1,  102,    2, 0x08 /* Private */,
      14,    1,  105,    2, 0x08 /* Private */,
      16,    0,  108,    2, 0x08 /* Private */,
      17,    0,  109,    2, 0x08 /* Private */,
      18,    0,  110,    2, 0x08 /* Private */,
      19,    0,  111,    2, 0x08 /* Private */,
      20,    0,  112,    2, 0x08 /* Private */,
      21,    0,  113,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      22, QMetaType::QReal, 0x00095103,

       0        // eod
};

void LoginWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        LoginWidget *_t = static_cast<LoginWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->loginRequested((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->loginSuccess(); break;
        case 2: _t->registerRequested(); break;
        case 3: _t->forgotPassword(); break;
        case 4: _t->onLoginClicked(); break;
        case 5: _t->onLoginSuccess(); break;
        case 6: _t->onLoginFailed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->onRememberChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->onServerChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->onRegisterClicked(); break;
        case 10: _t->onForgotPasswordClicked(); break;
        case 11: _t->onEyeButtonClicked(); break;
        case 12: _t->shakeWindow(); break;
        case 13: _t->showWelcomeMessage(); break;
        case 14: _t->fadeIn(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (LoginWidget::*_t)(const QString & , const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LoginWidget::loginRequested)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (LoginWidget::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LoginWidget::loginSuccess)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (LoginWidget::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LoginWidget::registerRequested)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (LoginWidget::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LoginWidget::forgotPassword)) {
                *result = 3;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        LoginWidget *_t = static_cast<LoginWidget *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< qreal*>(_v) = _t->opacity(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        LoginWidget *_t = static_cast<LoginWidget *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setOpacity(*reinterpret_cast< qreal*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

const QMetaObject LoginWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_LoginWidget.data,
      qt_meta_data_LoginWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *LoginWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LoginWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LoginWidget.stringdata0))
        return static_cast<void*>(const_cast< LoginWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int LoginWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 15;
    }
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void LoginWidget::loginRequested(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LoginWidget::loginSuccess()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void LoginWidget::registerRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void LoginWidget::forgotPassword()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
