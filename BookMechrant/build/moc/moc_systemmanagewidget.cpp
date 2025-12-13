/****************************************************************************
** Meta object code from reading C++ file 'systemmanagewidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../systemmanagewidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'systemmanagewidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SystemManageWidget_t {
    QByteArrayData data[18];
    char stringdata0[252];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SystemManageWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SystemManageWidget_t qt_meta_stringdata_SystemManageWidget = {
    {
QT_MOC_LITERAL(0, 0, 18), // "SystemManageWidget"
QT_MOC_LITERAL(1, 19, 14), // "onSaveShopInfo"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 21), // "onSaveCashierSettings"
QT_MOC_LITERAL(4, 57, 12), // "onBackupData"
QT_MOC_LITERAL(5, 70, 13), // "onRestoreData"
QT_MOC_LITERAL(6, 84, 16), // "onChangePassword"
QT_MOC_LITERAL(7, 101, 13), // "onChangeTheme"
QT_MOC_LITERAL(8, 115, 5), // "index"
QT_MOC_LITERAL(9, 121, 12), // "onExportData"
QT_MOC_LITERAL(10, 134, 11), // "onClearLogs"
QT_MOC_LITERAL(11, 146, 11), // "onTestPrint"
QT_MOC_LITERAL(12, 158, 15), // "onResetSettings"
QT_MOC_LITERAL(13, 174, 17), // "onSaveAllSettings"
QT_MOC_LITERAL(14, 192, 16), // "onImportSettings"
QT_MOC_LITERAL(15, 209, 13), // "onSystemCheck"
QT_MOC_LITERAL(16, 223, 16), // "onUpdateDatabase"
QT_MOC_LITERAL(17, 240, 11) // "resetStatus"

    },
    "SystemManageWidget\0onSaveShopInfo\0\0"
    "onSaveCashierSettings\0onBackupData\0"
    "onRestoreData\0onChangePassword\0"
    "onChangeTheme\0index\0onExportData\0"
    "onClearLogs\0onTestPrint\0onResetSettings\0"
    "onSaveAllSettings\0onImportSettings\0"
    "onSystemCheck\0onUpdateDatabase\0"
    "resetStatus"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SystemManageWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   89,    2, 0x08 /* Private */,
       3,    0,   90,    2, 0x08 /* Private */,
       4,    0,   91,    2, 0x08 /* Private */,
       5,    0,   92,    2, 0x08 /* Private */,
       6,    0,   93,    2, 0x08 /* Private */,
       7,    1,   94,    2, 0x08 /* Private */,
       9,    0,   97,    2, 0x08 /* Private */,
      10,    0,   98,    2, 0x08 /* Private */,
      11,    0,   99,    2, 0x08 /* Private */,
      12,    0,  100,    2, 0x08 /* Private */,
      13,    0,  101,    2, 0x08 /* Private */,
      14,    0,  102,    2, 0x08 /* Private */,
      15,    0,  103,    2, 0x08 /* Private */,
      16,    0,  104,    2, 0x08 /* Private */,
      17,    0,  105,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void SystemManageWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SystemManageWidget *_t = static_cast<SystemManageWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onSaveShopInfo(); break;
        case 1: _t->onSaveCashierSettings(); break;
        case 2: _t->onBackupData(); break;
        case 3: _t->onRestoreData(); break;
        case 4: _t->onChangePassword(); break;
        case 5: _t->onChangeTheme((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onExportData(); break;
        case 7: _t->onClearLogs(); break;
        case 8: _t->onTestPrint(); break;
        case 9: _t->onResetSettings(); break;
        case 10: _t->onSaveAllSettings(); break;
        case 11: _t->onImportSettings(); break;
        case 12: _t->onSystemCheck(); break;
        case 13: _t->onUpdateDatabase(); break;
        case 14: _t->resetStatus(); break;
        default: ;
        }
    }
}

const QMetaObject SystemManageWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SystemManageWidget.data,
      qt_meta_data_SystemManageWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SystemManageWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SystemManageWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SystemManageWidget.stringdata0))
        return static_cast<void*>(const_cast< SystemManageWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int SystemManageWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
