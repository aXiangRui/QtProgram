/****************************************************************************
** Meta object code from reading C++ file 'BookManageWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../BookManageWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BookManageWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_BookManageWidget_t {
    QByteArrayData data[14];
    char stringdata0[167];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BookManageWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BookManageWidget_t qt_meta_stringdata_BookManageWidget = {
    {
QT_MOC_LITERAL(0, 0, 16), // "BookManageWidget"
QT_MOC_LITERAL(1, 17, 9), // "onAddBook"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 10), // "onEditBook"
QT_MOC_LITERAL(4, 39, 12), // "onDeleteBook"
QT_MOC_LITERAL(5, 52, 8), // "onSearch"
QT_MOC_LITERAL(6, 61, 8), // "onImport"
QT_MOC_LITERAL(7, 70, 8), // "onExport"
QT_MOC_LITERAL(8, 79, 23), // "onTableSelectionChanged"
QT_MOC_LITERAL(9, 103, 10), // "onSaveBook"
QT_MOC_LITERAL(10, 114, 12), // "onCancelEdit"
QT_MOC_LITERAL(11, 127, 9), // "onRefresh"
QT_MOC_LITERAL(12, 137, 23), // "onCategoryFilterChanged"
QT_MOC_LITERAL(13, 161, 5) // "index"

    },
    "BookManageWidget\0onAddBook\0\0onEditBook\0"
    "onDeleteBook\0onSearch\0onImport\0onExport\0"
    "onTableSelectionChanged\0onSaveBook\0"
    "onCancelEdit\0onRefresh\0onCategoryFilterChanged\0"
    "index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BookManageWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    0,   71,    2, 0x08 /* Private */,
       5,    0,   72,    2, 0x08 /* Private */,
       6,    0,   73,    2, 0x08 /* Private */,
       7,    0,   74,    2, 0x08 /* Private */,
       8,    0,   75,    2, 0x08 /* Private */,
       9,    0,   76,    2, 0x08 /* Private */,
      10,    0,   77,    2, 0x08 /* Private */,
      11,    0,   78,    2, 0x08 /* Private */,
      12,    1,   79,    2, 0x08 /* Private */,

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
    QMetaType::Void, QMetaType::Int,   13,

       0        // eod
};

void BookManageWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        BookManageWidget *_t = static_cast<BookManageWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onAddBook(); break;
        case 1: _t->onEditBook(); break;
        case 2: _t->onDeleteBook(); break;
        case 3: _t->onSearch(); break;
        case 4: _t->onImport(); break;
        case 5: _t->onExport(); break;
        case 6: _t->onTableSelectionChanged(); break;
        case 7: _t->onSaveBook(); break;
        case 8: _t->onCancelEdit(); break;
        case 9: _t->onRefresh(); break;
        case 10: _t->onCategoryFilterChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject BookManageWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_BookManageWidget.data,
      qt_meta_data_BookManageWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *BookManageWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BookManageWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_BookManageWidget.stringdata0))
        return static_cast<void*>(const_cast< BookManageWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int BookManageWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
