/****************************************************************************
** Meta object code from reading C++ file 'DashboardWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../DashboardWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DashboardWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DashboardWidget_t {
    QByteArrayData data[12];
    char stringdata0[188];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DashboardWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DashboardWidget_t qt_meta_stringdata_DashboardWidget = {
    {
QT_MOC_LITERAL(0, 0, 15), // "DashboardWidget"
QT_MOC_LITERAL(1, 16, 15), // "navigateToBooks"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 16), // "navigateToOrders"
QT_MOC_LITERAL(4, 50, 17), // "navigateToReports"
QT_MOC_LITERAL(5, 68, 19), // "navigateToInventory"
QT_MOC_LITERAL(6, 88, 11), // "refreshData"
QT_MOC_LITERAL(7, 100, 13), // "onPrintReport"
QT_MOC_LITERAL(8, 114, 16), // "onInventoryCheck"
QT_MOC_LITERAL(9, 131, 18), // "updateRecentOrders"
QT_MOC_LITERAL(10, 150, 21), // "updateInventoryAlerts"
QT_MOC_LITERAL(11, 172, 15) // "updateChartData"

    },
    "DashboardWidget\0navigateToBooks\0\0"
    "navigateToOrders\0navigateToReports\0"
    "navigateToInventory\0refreshData\0"
    "onPrintReport\0onInventoryCheck\0"
    "updateRecentOrders\0updateInventoryAlerts\0"
    "updateChartData"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DashboardWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,
       3,    0,   65,    2, 0x06 /* Public */,
       4,    0,   66,    2, 0x06 /* Public */,
       5,    0,   67,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   68,    2, 0x08 /* Private */,
       7,    0,   69,    2, 0x08 /* Private */,
       8,    0,   70,    2, 0x08 /* Private */,
       9,    0,   71,    2, 0x08 /* Private */,
      10,    0,   72,    2, 0x08 /* Private */,
      11,    0,   73,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void DashboardWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DashboardWidget *_t = static_cast<DashboardWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->navigateToBooks(); break;
        case 1: _t->navigateToOrders(); break;
        case 2: _t->navigateToReports(); break;
        case 3: _t->navigateToInventory(); break;
        case 4: _t->refreshData(); break;
        case 5: _t->onPrintReport(); break;
        case 6: _t->onInventoryCheck(); break;
        case 7: _t->updateRecentOrders(); break;
        case 8: _t->updateInventoryAlerts(); break;
        case 9: _t->updateChartData(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DashboardWidget::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DashboardWidget::navigateToBooks)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DashboardWidget::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DashboardWidget::navigateToOrders)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (DashboardWidget::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DashboardWidget::navigateToReports)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (DashboardWidget::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DashboardWidget::navigateToInventory)) {
                *result = 3;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject DashboardWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DashboardWidget.data,
      qt_meta_data_DashboardWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DashboardWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DashboardWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DashboardWidget.stringdata0))
        return static_cast<void*>(const_cast< DashboardWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int DashboardWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void DashboardWidget::navigateToBooks()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DashboardWidget::navigateToOrders()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DashboardWidget::navigateToReports()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void DashboardWidget::navigateToInventory()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
