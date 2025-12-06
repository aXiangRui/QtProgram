/********************************************************************************
** Form generated from reading UI file 'serverwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERWINDOW_H
#define UI_SERVERWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ServerWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *tcpStatusLabel;
    QPushButton *startTcpBtn;
    QLabel *httpStatusLabel;
    QPushButton *startHttpBtn;
    QLabel *wsStatusLabel;
    QPushButton *startWsBtn;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QTextEdit *logEdit;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ServerWindow)
    {
        if (ServerWindow->objectName().isEmpty())
            ServerWindow->setObjectName(QStringLiteral("ServerWindow"));
        ServerWindow->resize(800, 600);
        centralWidget = new QWidget(ServerWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        tcpStatusLabel = new QLabel(groupBox);
        tcpStatusLabel->setObjectName(QStringLiteral("tcpStatusLabel"));

        gridLayout->addWidget(tcpStatusLabel, 0, 0, 1, 1);

        startTcpBtn = new QPushButton(groupBox);
        startTcpBtn->setObjectName(QStringLiteral("startTcpBtn"));

        gridLayout->addWidget(startTcpBtn, 0, 1, 1, 1);

        httpStatusLabel = new QLabel(groupBox);
        httpStatusLabel->setObjectName(QStringLiteral("httpStatusLabel"));

        gridLayout->addWidget(httpStatusLabel, 1, 0, 1, 1);

        startHttpBtn = new QPushButton(groupBox);
        startHttpBtn->setObjectName(QStringLiteral("startHttpBtn"));

        gridLayout->addWidget(startHttpBtn, 1, 1, 1, 1);

        wsStatusLabel = new QLabel(groupBox);
        wsStatusLabel->setObjectName(QStringLiteral("wsStatusLabel"));

        gridLayout->addWidget(wsStatusLabel, 2, 0, 1, 1);

        startWsBtn = new QPushButton(groupBox);
        startWsBtn->setObjectName(QStringLiteral("startWsBtn"));

        gridLayout->addWidget(startWsBtn, 2, 1, 1, 1);


        verticalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        logEdit = new QTextEdit(groupBox_2);
        logEdit->setObjectName(QStringLiteral("logEdit"));
        logEdit->setReadOnly(true);

        verticalLayout_2->addWidget(logEdit);


        verticalLayout->addWidget(groupBox_2);

        ServerWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ServerWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 800, 23));
        ServerWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(ServerWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ServerWindow->setStatusBar(statusBar);

        retranslateUi(ServerWindow);

        QMetaObject::connectSlotsByName(ServerWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ServerWindow)
    {
        ServerWindow->setWindowTitle(QApplication::translate("ServerWindow", "\345\244\232\345\215\217\350\256\256\346\234\215\345\212\241\345\231\250 v1.0", Q_NULLPTR));
        groupBox->setTitle(QApplication::translate("ServerWindow", "\346\234\215\345\212\241\346\216\247\345\210\266", Q_NULLPTR));
        tcpStatusLabel->setText(QApplication::translate("ServerWindow", "TCP\346\234\215\345\212\241\357\274\232\345\267\262\345\201\234\346\255\242", Q_NULLPTR));
        startTcpBtn->setText(QApplication::translate("ServerWindow", "\345\220\257\345\212\250TCP", Q_NULLPTR));
        httpStatusLabel->setText(QApplication::translate("ServerWindow", "HTTP\346\234\215\345\212\241\357\274\232\345\267\262\345\201\234\346\255\242", Q_NULLPTR));
        startHttpBtn->setText(QApplication::translate("ServerWindow", "\345\220\257\345\212\250HTTP", Q_NULLPTR));
        wsStatusLabel->setText(QApplication::translate("ServerWindow", "WebSocket\346\234\215\345\212\241\357\274\232\345\267\262\345\201\234\346\255\242", Q_NULLPTR));
        startWsBtn->setText(QApplication::translate("ServerWindow", "\345\220\257\345\212\250WebSocket", Q_NULLPTR));
        groupBox_2->setTitle(QApplication::translate("ServerWindow", "\350\277\220\350\241\214\346\227\245\345\277\227", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ServerWindow: public Ui_ServerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERWINDOW_H
