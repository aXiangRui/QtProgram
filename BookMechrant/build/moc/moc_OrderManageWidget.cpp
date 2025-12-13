/****************************************************************************
** Meta object code from reading C++ file 'OrderManageWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../OrderManageWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OrderManageWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OrderManageWidget_t {
    QByteArrayData data[22];
    char stringdata0[340];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OrderManageWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OrderManageWidget_t qt_meta_stringdata_OrderManageWidget = {
    {
QT_MOC_LITERAL(0, 0, 17), // "OrderManageWidget"
QT_MOC_LITERAL(1, 18, 12), // "onTabChanged"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 5), // "index"
QT_MOC_LITERAL(4, 38, 20), // "onOpenNewOrderDialog"
QT_MOC_LITERAL(5, 59, 14), // "onExportOrders"
QT_MOC_LITERAL(6, 74, 9), // "onRefresh"
QT_MOC_LITERAL(7, 84, 8), // "onSearch"
QT_MOC_LITERAL(8, 93, 18), // "onShowOrderDetails"
QT_MOC_LITERAL(9, 112, 23), // "onTableSelectionChanged"
QT_MOC_LITERAL(10, 136, 15), // "onFilterChanged"
QT_MOC_LITERAL(11, 152, 20), // "onAmountRangeChanged"
QT_MOC_LITERAL(12, 173, 18), // "onExportCurrentTab"
QT_MOC_LITERAL(13, 192, 14), // "onClearFilters"
QT_MOC_LITERAL(14, 207, 13), // "onDeleteOrder"
QT_MOC_LITERAL(15, 221, 11), // "onclearCart"
QT_MOC_LITERAL(16, 233, 11), // "onAddToCart"
QT_MOC_LITERAL(17, 245, 16), // "onRemoveFromCart"
QT_MOC_LITERAL(18, 262, 20), // "onUpdateCartQuantity"
QT_MOC_LITERAL(19, 283, 17), // "onConfirmNewOrder"
QT_MOC_LITERAL(20, 301, 16), // "onCancelNewOrder"
QT_MOC_LITERAL(21, 318, 21) // "onSelectPaymentMethod"

    },
    "OrderManageWidget\0onTabChanged\0\0index\0"
    "onOpenNewOrderDialog\0onExportOrders\0"
    "onRefresh\0onSearch\0onShowOrderDetails\0"
    "onTableSelectionChanged\0onFilterChanged\0"
    "onAmountRangeChanged\0onExportCurrentTab\0"
    "onClearFilters\0onDeleteOrder\0onclearCart\0"
    "onAddToCart\0onRemoveFromCart\0"
    "onUpdateCartQuantity\0onConfirmNewOrder\0"
    "onCancelNewOrder\0onSelectPaymentMethod"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OrderManageWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,  109,    2, 0x08 /* Private */,
       4,    0,  112,    2, 0x08 /* Private */,
       5,    0,  113,    2, 0x08 /* Private */,
       6,    0,  114,    2, 0x08 /* Private */,
       7,    0,  115,    2, 0x08 /* Private */,
       8,    0,  116,    2, 0x08 /* Private */,
       9,    0,  117,    2, 0x08 /* Private */,
      10,    0,  118,    2, 0x08 /* Private */,
      11,    0,  119,    2, 0x08 /* Private */,
      12,    0,  120,    2, 0x08 /* Private */,
      13,    0,  121,    2, 0x08 /* Private */,
      14,    0,  122,    2, 0x08 /* Private */,
      15,    0,  123,    2, 0x08 /* Private */,
      16,    0,  124,    2, 0x08 /* Private */,
      17,    0,  125,    2, 0x08 /* Private */,
      18,    0,  126,    2, 0x08 /* Private */,
      19,    0,  127,    2, 0x08 /* Private */,
      20,    0,  128,    2, 0x08 /* Private */,
      21,    0,  129,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
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

void OrderManageWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OrderManageWidget *_t = static_cast<OrderManageWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onTabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onOpenNewOrderDialog(); break;
        case 2: _t->onExportOrders(); break;
        case 3: _t->onRefresh(); break;
        case 4: _t->onSearch(); break;
        case 5: _t->onShowOrderDetails(); break;
        case 6: _t->onTableSelectionChanged(); break;
        case 7: _t->onFilterChanged(); break;
        case 8: _t->onAmountRangeChanged(); break;
        case 9: _t->onExportCurrentTab(); break;
        case 10: _t->onClearFilters(); break;
        case 11: _t->onDeleteOrder(); break;
        case 12: _t->onclearCart(); break;
        case 13: _t->onAddToCart(); break;
        case 14: _t->onRemoveFromCart(); break;
        case 15: _t->onUpdateCartQuantity(); break;
        case 16: _t->onConfirmNewOrder(); break;
        case 17: _t->onCancelNewOrder(); break;
        case 18: _t->onSelectPaymentMethod(); break;
        default: ;
        }
    }
}

const QMetaObject OrderManageWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_OrderManageWidget.data,
      qt_meta_data_OrderManageWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *OrderManageWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OrderManageWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OrderManageWidget.stringdata0))
        return static_cast<void*>(const_cast< OrderManageWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int OrderManageWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
