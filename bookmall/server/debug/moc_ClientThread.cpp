/****************************************************************************
** Meta object code from reading C++ file 'ClientThread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ClientThread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ClientThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ClientThread_t {
    QByteArrayData data[13];
    char stringdata0[149];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ClientThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ClientThread_t qt_meta_stringdata_ClientThread = {
    {
QT_MOC_LITERAL(0, 0, 12), // "ClientThread"
QT_MOC_LITERAL(1, 13, 12), // "disconnected"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 7), // "qintptr"
QT_MOC_LITERAL(4, 35, 16), // "socketDescriptor"
QT_MOC_LITERAL(5, 52, 12), // "dataReceived"
QT_MOC_LITERAL(6, 65, 4), // "data"
QT_MOC_LITERAL(7, 70, 13), // "errorOccurred"
QT_MOC_LITERAL(8, 84, 23), // "QTcpSocket::SocketError"
QT_MOC_LITERAL(9, 108, 5), // "error"
QT_MOC_LITERAL(10, 114, 11), // "onReadyRead"
QT_MOC_LITERAL(11, 126, 14), // "onDisconnected"
QT_MOC_LITERAL(12, 141, 7) // "onError"

    },
    "ClientThread\0disconnected\0\0qintptr\0"
    "socketDescriptor\0dataReceived\0data\0"
    "errorOccurred\0QTcpSocket::SocketError\0"
    "error\0onReadyRead\0onDisconnected\0"
    "onError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ClientThread[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       5,    2,   47,    2, 0x06 /* Public */,
       7,    1,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   55,    2, 0x08 /* Private */,
      11,    0,   56,    2, 0x08 /* Private */,
      12,    1,   57,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    6,
    QMetaType::Void, 0x80000000 | 8,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,

       0        // eod
};

void ClientThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ClientThread *_t = static_cast<ClientThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected((*reinterpret_cast< qintptr(*)>(_a[1]))); break;
        case 1: _t->dataReceived((*reinterpret_cast< qintptr(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 2: _t->errorOccurred((*reinterpret_cast< QTcpSocket::SocketError(*)>(_a[1]))); break;
        case 3: _t->onReadyRead(); break;
        case 4: _t->onDisconnected(); break;
        case 5: _t->onError((*reinterpret_cast< QTcpSocket::SocketError(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ClientThread::*_t)(qintptr );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ClientThread::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ClientThread::*_t)(qintptr , const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ClientThread::dataReceived)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (ClientThread::*_t)(QTcpSocket::SocketError );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ClientThread::errorOccurred)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject ClientThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_ClientThread.data,
      qt_meta_data_ClientThread,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ClientThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ClientThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ClientThread.stringdata0))
        return static_cast<void*>(const_cast< ClientThread*>(this));
    return QThread::qt_metacast(_clname);
}

int ClientThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ClientThread::disconnected(qintptr _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ClientThread::dataReceived(qintptr _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ClientThread::errorOccurred(QTcpSocket::SocketError _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
