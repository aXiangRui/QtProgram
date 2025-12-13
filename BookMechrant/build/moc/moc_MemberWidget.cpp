/****************************************************************************
** Meta object code from reading C++ file 'MemberWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../MemberWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MemberWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MemberWidget_t {
    QByteArrayData data[18];
    char stringdata0[231];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MemberWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MemberWidget_t qt_meta_stringdata_MemberWidget = {
    {
QT_MOC_LITERAL(0, 0, 12), // "MemberWidget"
QT_MOC_LITERAL(1, 13, 11), // "onAddMember"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 12), // "onEditMember"
QT_MOC_LITERAL(4, 39, 14), // "onDeleteMember"
QT_MOC_LITERAL(5, 54, 10), // "onRecharge"
QT_MOC_LITERAL(6, 65, 8), // "onSearch"
QT_MOC_LITERAL(7, 74, 13), // "onSendMessage"
QT_MOC_LITERAL(8, 88, 15), // "onExportMembers"
QT_MOC_LITERAL(9, 104, 9), // "onRefresh"
QT_MOC_LITERAL(10, 114, 12), // "onSaveMember"
QT_MOC_LITERAL(11, 127, 12), // "onCancelEdit"
QT_MOC_LITERAL(12, 140, 23), // "onTableSelectionChanged"
QT_MOC_LITERAL(13, 164, 20), // "onLevelFilterChanged"
QT_MOC_LITERAL(14, 185, 5), // "index"
QT_MOC_LITERAL(15, 191, 18), // "onRechargeComplete"
QT_MOC_LITERAL(16, 210, 6), // "amount"
QT_MOC_LITERAL(17, 217, 13) // "paymentMethod"

    },
    "MemberWidget\0onAddMember\0\0onEditMember\0"
    "onDeleteMember\0onRecharge\0onSearch\0"
    "onSendMessage\0onExportMembers\0onRefresh\0"
    "onSaveMember\0onCancelEdit\0"
    "onTableSelectionChanged\0onLevelFilterChanged\0"
    "index\0onRechargeComplete\0amount\0"
    "paymentMethod"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MemberWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    0,   80,    2, 0x08 /* Private */,
       4,    0,   81,    2, 0x08 /* Private */,
       5,    0,   82,    2, 0x08 /* Private */,
       6,    0,   83,    2, 0x08 /* Private */,
       7,    0,   84,    2, 0x08 /* Private */,
       8,    0,   85,    2, 0x08 /* Private */,
       9,    0,   86,    2, 0x08 /* Private */,
      10,    0,   87,    2, 0x08 /* Private */,
      11,    0,   88,    2, 0x08 /* Private */,
      12,    0,   89,    2, 0x08 /* Private */,
      13,    1,   90,    2, 0x08 /* Private */,
      15,    2,   93,    2, 0x08 /* Private */,

 // slots: parameters
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
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Double, QMetaType::QString,   16,   17,

       0        // eod
};

void MemberWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MemberWidget *_t = static_cast<MemberWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onAddMember(); break;
        case 1: _t->onEditMember(); break;
        case 2: _t->onDeleteMember(); break;
        case 3: _t->onRecharge(); break;
        case 4: _t->onSearch(); break;
        case 5: _t->onSendMessage(); break;
        case 6: _t->onExportMembers(); break;
        case 7: _t->onRefresh(); break;
        case 8: _t->onSaveMember(); break;
        case 9: _t->onCancelEdit(); break;
        case 10: _t->onTableSelectionChanged(); break;
        case 11: _t->onLevelFilterChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->onRechargeComplete((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject MemberWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MemberWidget.data,
      qt_meta_data_MemberWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *MemberWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MemberWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MemberWidget.stringdata0))
        return static_cast<void*>(const_cast< MemberWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int MemberWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
struct qt_meta_stringdata_RechargeDialog_t {
    QByteArrayData data[8];
    char stringdata0[113];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RechargeDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RechargeDialog_t qt_meta_stringdata_RechargeDialog = {
    {
QT_MOC_LITERAL(0, 0, 14), // "RechargeDialog"
QT_MOC_LITERAL(1, 15, 17), // "rechargeRequested"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 6), // "amount"
QT_MOC_LITERAL(4, 41, 13), // "paymentMethod"
QT_MOC_LITERAL(5, 55, 17), // "onRechargeClicked"
QT_MOC_LITERAL(6, 73, 21), // "onAmountButtonClicked"
QT_MOC_LITERAL(7, 95, 17) // "updateTotalAmount"

    },
    "RechargeDialog\0rechargeRequested\0\0"
    "amount\0paymentMethod\0onRechargeClicked\0"
    "onAmountButtonClicked\0updateTotalAmount"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RechargeDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   39,    2, 0x08 /* Private */,
       6,    0,   40,    2, 0x08 /* Private */,
       7,    0,   41,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double, QMetaType::QString,    3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void RechargeDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RechargeDialog *_t = static_cast<RechargeDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->rechargeRequested((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->onRechargeClicked(); break;
        case 2: _t->onAmountButtonClicked(); break;
        case 3: _t->updateTotalAmount(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (RechargeDialog::*_t)(double , const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RechargeDialog::rechargeRequested)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject RechargeDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_RechargeDialog.data,
      qt_meta_data_RechargeDialog,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *RechargeDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RechargeDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RechargeDialog.stringdata0))
        return static_cast<void*>(const_cast< RechargeDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int RechargeDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void RechargeDialog::rechargeRequested(double _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
