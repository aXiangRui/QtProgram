/********************************************************************************
** Form generated from reading UI file 'clientwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENTWINDOW_H
#define UI_CLIENTWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ClientWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *serverIpEdit;
    QLabel *label_2;
    QLineEdit *tcpPortEdit;
    QLabel *label_3;
    QLineEdit *wsPortEdit;
    QTabWidget *tabWidget;
    QWidget *tcpTab;
    QVBoxLayout *verticalLayout_2;
    QPushButton *tcpConnectBtn;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *filePathEdit;
    QPushButton *selectFileBtn;
    QPushButton *sendFileBtn;
    QWidget *httpTab;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *getBooksBtn;
    QPushButton *getUserBtn;
    QPushButton *getCartBtn;
    QPushButton *getOrderBtn;
    QPushButton *getCollectBtn;
    QTextEdit *httpResponseEdit;
    QWidget *wsTab;
    QVBoxLayout *verticalLayout_4;
    QPushButton *wsConnectBtn;
    QTextEdit *wsMsgEdit;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *wsMsgInput;
    QPushButton *sendWsMsgBtn;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_5;
    QTextEdit *logEdit;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ClientWindow)
    {
        if (ClientWindow->objectName().isEmpty())
            ClientWindow->setObjectName(QStringLiteral("ClientWindow"));
        ClientWindow->resize(900, 700);
        centralWidget = new QWidget(ClientWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        serverIpEdit = new QLineEdit(groupBox);
        serverIpEdit->setObjectName(QStringLiteral("serverIpEdit"));

        horizontalLayout->addWidget(serverIpEdit);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout->addWidget(label_2);

        tcpPortEdit = new QLineEdit(groupBox);
        tcpPortEdit->setObjectName(QStringLiteral("tcpPortEdit"));

        horizontalLayout->addWidget(tcpPortEdit);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        horizontalLayout->addWidget(label_3);

        wsPortEdit = new QLineEdit(groupBox);
        wsPortEdit->setObjectName(QStringLiteral("wsPortEdit"));

        horizontalLayout->addWidget(wsPortEdit);


        verticalLayout->addWidget(groupBox);

        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tcpTab = new QWidget();
        tcpTab->setObjectName(QStringLiteral("tcpTab"));
        verticalLayout_2 = new QVBoxLayout(tcpTab);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        tcpConnectBtn = new QPushButton(tcpTab);
        tcpConnectBtn->setObjectName(QStringLiteral("tcpConnectBtn"));

        verticalLayout_2->addWidget(tcpConnectBtn);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        filePathEdit = new QLineEdit(tcpTab);
        filePathEdit->setObjectName(QStringLiteral("filePathEdit"));

        horizontalLayout_2->addWidget(filePathEdit);

        selectFileBtn = new QPushButton(tcpTab);
        selectFileBtn->setObjectName(QStringLiteral("selectFileBtn"));

        horizontalLayout_2->addWidget(selectFileBtn);

        sendFileBtn = new QPushButton(tcpTab);
        sendFileBtn->setObjectName(QStringLiteral("sendFileBtn"));

        horizontalLayout_2->addWidget(sendFileBtn);


        verticalLayout_2->addLayout(horizontalLayout_2);

        tabWidget->addTab(tcpTab, QString());
        httpTab = new QWidget();
        httpTab->setObjectName(QStringLiteral("httpTab"));
        verticalLayout_3 = new QVBoxLayout(httpTab);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        getBooksBtn = new QPushButton(httpTab);
        getBooksBtn->setObjectName(QStringLiteral("getBooksBtn"));

        horizontalLayout_3->addWidget(getBooksBtn);

        getUserBtn = new QPushButton(httpTab);
        getUserBtn->setObjectName(QStringLiteral("getUserBtn"));

        horizontalLayout_3->addWidget(getUserBtn);

        getCartBtn = new QPushButton(httpTab);
        getCartBtn->setObjectName(QStringLiteral("getCartBtn"));

        horizontalLayout_3->addWidget(getCartBtn);

        getOrderBtn = new QPushButton(httpTab);
        getOrderBtn->setObjectName(QStringLiteral("getOrderBtn"));

        horizontalLayout_3->addWidget(getOrderBtn);

        getCollectBtn = new QPushButton(httpTab);
        getCollectBtn->setObjectName(QStringLiteral("getCollectBtn"));

        horizontalLayout_3->addWidget(getCollectBtn);


        verticalLayout_3->addLayout(horizontalLayout_3);

        httpResponseEdit = new QTextEdit(httpTab);
        httpResponseEdit->setObjectName(QStringLiteral("httpResponseEdit"));
        httpResponseEdit->setReadOnly(true);

        verticalLayout_3->addWidget(httpResponseEdit);

        tabWidget->addTab(httpTab, QString());
        wsTab = new QWidget();
        wsTab->setObjectName(QStringLiteral("wsTab"));
        verticalLayout_4 = new QVBoxLayout(wsTab);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        wsConnectBtn = new QPushButton(wsTab);
        wsConnectBtn->setObjectName(QStringLiteral("wsConnectBtn"));

        verticalLayout_4->addWidget(wsConnectBtn);

        wsMsgEdit = new QTextEdit(wsTab);
        wsMsgEdit->setObjectName(QStringLiteral("wsMsgEdit"));
        wsMsgEdit->setReadOnly(true);

        verticalLayout_4->addWidget(wsMsgEdit);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        wsMsgInput = new QLineEdit(wsTab);
        wsMsgInput->setObjectName(QStringLiteral("wsMsgInput"));

        horizontalLayout_4->addWidget(wsMsgInput);

        sendWsMsgBtn = new QPushButton(wsTab);
        sendWsMsgBtn->setObjectName(QStringLiteral("sendWsMsgBtn"));

        horizontalLayout_4->addWidget(sendWsMsgBtn);


        verticalLayout_4->addLayout(horizontalLayout_4);

        tabWidget->addTab(wsTab, QString());

        verticalLayout->addWidget(tabWidget);

        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        verticalLayout_5 = new QVBoxLayout(groupBox_2);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        logEdit = new QTextEdit(groupBox_2);
        logEdit->setObjectName(QStringLiteral("logEdit"));
        logEdit->setReadOnly(true);

        verticalLayout_5->addWidget(logEdit);


        verticalLayout->addWidget(groupBox_2);

        ClientWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ClientWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 900, 23));
        ClientWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(ClientWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ClientWindow->setStatusBar(statusBar);

        retranslateUi(ClientWindow);

        QMetaObject::connectSlotsByName(ClientWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ClientWindow)
    {
        ClientWindow->setWindowTitle(QApplication::translate("ClientWindow", "\345\244\232\345\215\217\350\256\256\345\256\242\346\210\267\347\253\257 v1.0", Q_NULLPTR));
        groupBox->setTitle(QApplication::translate("ClientWindow", "\346\234\215\345\212\241\345\231\250\351\205\215\347\275\256", Q_NULLPTR));
        label->setText(QApplication::translate("ClientWindow", "\346\234\215\345\212\241\345\231\250IP\357\274\232", Q_NULLPTR));
        serverIpEdit->setText(QApplication::translate("ClientWindow", "127.0.0.1", Q_NULLPTR));
        label_2->setText(QApplication::translate("ClientWindow", "TCP\347\253\257\345\217\243\357\274\232", Q_NULLPTR));
        tcpPortEdit->setText(QApplication::translate("ClientWindow", "8888", Q_NULLPTR));
        label_3->setText(QApplication::translate("ClientWindow", "WS\347\253\257\345\217\243\357\274\232", Q_NULLPTR));
        wsPortEdit->setText(QApplication::translate("ClientWindow", "9090", Q_NULLPTR));
        tcpConnectBtn->setText(QApplication::translate("ClientWindow", "\350\277\236\346\216\245TCP\346\234\215\345\212\241\345\231\250", Q_NULLPTR));
        filePathEdit->setPlaceholderText(QApplication::translate("ClientWindow", "\351\200\211\344\270\255\347\232\204\346\226\207\344\273\266\350\267\257\345\276\204", Q_NULLPTR));
        selectFileBtn->setText(QApplication::translate("ClientWindow", "\351\200\211\346\213\251\346\226\207\344\273\266", Q_NULLPTR));
        sendFileBtn->setText(QApplication::translate("ClientWindow", "\345\217\221\351\200\201\346\226\207\344\273\266", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tcpTab), QApplication::translate("ClientWindow", "TCP\346\226\207\344\273\266\344\274\240\350\276\223", Q_NULLPTR));
        getBooksBtn->setText(QApplication::translate("ClientWindow", "\345\233\276\344\271\246\346\265\217\350\247\210", Q_NULLPTR));
        getUserBtn->setText(QApplication::translate("ClientWindow", "\347\224\250\346\210\267\347\256\241\347\220\206", Q_NULLPTR));
        getCartBtn->setText(QApplication::translate("ClientWindow", "\350\264\255\347\211\251\350\275\246", Q_NULLPTR));
        getOrderBtn->setText(QApplication::translate("ClientWindow", "\350\256\242\345\215\225", Q_NULLPTR));
        getCollectBtn->setText(QApplication::translate("ClientWindow", "\346\224\266\350\227\217", Q_NULLPTR));
        httpResponseEdit->setPlaceholderText(QApplication::translate("ClientWindow", "HTTP\345\223\215\345\272\224\345\206\205\345\256\271\345\260\206\346\230\276\347\244\272\345\234\250\350\277\231\351\207\214", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(httpTab), QApplication::translate("ClientWindow", "HTTP\344\270\232\345\212\241\350\257\267\346\261\202", Q_NULLPTR));
        wsConnectBtn->setText(QApplication::translate("ClientWindow", "\350\277\236\346\216\245WebSocket\346\234\215\345\212\241\345\231\250", Q_NULLPTR));
        wsMsgEdit->setPlaceholderText(QApplication::translate("ClientWindow", "\346\266\210\346\201\257\350\256\260\345\275\225\345\260\206\346\230\276\347\244\272\345\234\250\350\277\231\351\207\214", Q_NULLPTR));
        wsMsgInput->setPlaceholderText(QApplication::translate("ClientWindow", "\350\276\223\345\205\245\350\246\201\345\217\221\351\200\201\347\232\204\346\266\210\346\201\257", Q_NULLPTR));
        sendWsMsgBtn->setText(QApplication::translate("ClientWindow", "\345\217\221\351\200\201\346\266\210\346\201\257", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(wsTab), QApplication::translate("ClientWindow", "WebSocket\345\256\236\346\227\266\351\200\232\344\277\241", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("ClientWindow", "\346\223\215\344\275\234\346\227\245\345\277\227", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ClientWindow: public Ui_ClientWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENTWINDOW_H
